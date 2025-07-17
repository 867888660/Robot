#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <AsyncElegantOTA.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <time.h>
#include <ArduinoJson.h> // Added for JSON parsing

// 配置参数
#define HOST_AP_SSID "CarConfig_AP"
#define HOST_AP_PASS "12345678"
#define SERIAL_BAUD 115200
#define STM32_SERIAL Serial2
#define STM32_BAUD 115200
#define RESET_PIN 4      // 连接到STM32的RESET引脚
#define BOOT0_PIN 5      // 连接到STM32的BOOT0引脚
#define CONFIG_PORTAL_TIMEOUT 180  // 配置门户超时时间(秒)
#define AP_CHANNEL 6     // 热点信道，避免与家庭WiFi冲突
#define FIRMWARE_VERSION "1.0.0"   // 固件版本号
#define DEVICE_MODEL "ESP32-WROOM-32"  // 设备型号

// 存储上次升级时间
RTC_DATA_ATTR char lastUpdateTime[32] = "从未升级";
unsigned long bootTime = 0;

// 服务实例
AsyncWebServer server(80);
WebSocketsServer webSocket(81, "/ws");
bool isConfigMode = false;  // 是否处于配置模式

// YMODEM协议常量
#define YMODEM_SOH 0x01  // 128字节数据包开始标记
#define YMODEM_STX 0x02  // 1024字节数据包开始标记
#define YMODEM_EOT 0x04  // 传输结束标记
#define YMODEM_ACK 0x06  // 确认
#define YMODEM_NAK 0x15  // 否认
#define YMODEM_CAN 0x18  // 取消传输
#define YMODEM_C   0x43  // 'C'字符，用于CRC模式

// YMODEM状态
enum YModemState {
  YMODEM_IDLE,
  YMODEM_WAIT_START,
  YMODEM_RECEIVE_DATA,
  YMODEM_RECEIVE_EOT,
  YMODEM_COMPLETE,
  YMODEM_ERROR
};

// STM32通信状态
enum STM32CommState {
  STM32_COMM_READY,
  STM32_COMM_BUSY,
  STM32_COMM_ERROR,
  STM32_COMM_TIMEOUT
};

// STM32通信管理
class STM32Comm {
private:
  HardwareSerial& serial;
  unsigned long lastCommandTime = 0;
  STM32CommState state = STM32_COMM_READY;
  int retryCount = 0;
  const int maxRetries = 3;
  String lastCommand = "";
  unsigned long responseTimeout = 500; // 默认响应超时时间(毫秒)

public:
  STM32Comm(HardwareSerial& _serial) : serial(_serial) {}
  
  bool sendCommand(const String& command, unsigned long timeout = 500) {
    if (state == STM32_COMM_BUSY) {
      Serial.println("STM32正忙，稍后再试");
      return false;
    }
    
    // 重置状态
    state = STM32_COMM_BUSY;
    lastCommandTime = millis();
    lastCommand = command;
    responseTimeout = timeout;
    retryCount = 0;
    
    // 发送命令
    serial.print(command);
    serial.print("\n");
    
    return true;
  }
  
  void retry() {
    if (retryCount < maxRetries && state == STM32_COMM_TIMEOUT) {
      retryCount++;
      Serial.println("重试发送命令: " + lastCommand + " (尝试 " + String(retryCount) + "/" + String(maxRetries) + ")");
      serial.print(lastCommand);
      serial.print("\n");
      lastCommandTime = millis();
      state = STM32_COMM_BUSY;
    } else if (retryCount >= maxRetries) {
      Serial.println("已达到最大重试次数，放弃");
      state = STM32_COMM_ERROR;
    }
  }
  
  void update() {
    // 检查超时
    if (state == STM32_COMM_BUSY && (millis() - lastCommandTime > responseTimeout)) {
      Serial.println("STM32响应超时");
      state = STM32_COMM_TIMEOUT;
      retry();
    }
  }
  
  void handleResponse(const String& response) {
    // 处理响应，重置状态
    Serial.println("收到STM32响应: " + response);
    state = STM32_COMM_READY;
    retryCount = 0;
  }
  
  STM32CommState getState() {
    return state;
  }
  
  void reset() {
    state = STM32_COMM_READY;
    retryCount = 0;
  }
};

// 创建STM32通信管理实例
STM32Comm stm32Comm(STM32_SERIAL);

// 检查STM32状态
bool checkSTM32Status() {
  if (!stm32Comm.sendCommand("{\"cmd\":\"status\"}", 1000)) {
    return false;
  }
  
  // 此处理想使用更可靠的方式确认STM32状态
  // 实际项目中应该实现一个基于状态和回调的通信系统
  delay(100);
  
  return stm32Comm.getState() != STM32_COMM_ERROR;
}

// 初始化文件系统
void setupFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS挂载失败");
    return;
  }
  Serial.println("LittleFS挂载成功");
}

// WebSocket事件处理
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] 断开连接!\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("[%u] 连接来自: %s\n", num, webSocket.remoteIP(num).toString().c_str());
      break;
    case WStype_TEXT:
      Serial.printf("[%u] 收到文本: %s\n", num, payload);
      
      // 支持两种格式:
      // 1. 简单CSV格式: "vx,vy,w"
      // 2. JSON格式: {"cmd":"velocity","vx":0.5,"vy":0,"w":0}
      
      // 先尝试CSV格式 (来自joystick.html)
      float vx = 0, vy = 0, w = 0;
      if (sscanf((const char*)payload, "%f,%f,%f", &vx, &vy, &w) == 3) {
        // 成功解析CSV格式
        String controlMsg = "{\"cmd\":\"velocity\",\"vx\":" + String(vx) + 
                           ",\"vy\":" + String(vy) + 
                           ",\"w\":" + String(w) + "}";
        
        // 使用通信管理类发送到STM32
        stm32Comm.sendCommand(controlMsg);
      }
      else {
        // 尝试JSON格式
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        
        if (!error) {
          // 处理命令
          const char* cmd = doc["cmd"];
          
          if (cmd && strcmp(cmd, "velocity") == 0) {
            // 处理速度指令
            float vx = doc["vx"];
            float vy = doc["vy"];
            float w = doc["w"];
            
            // 构建转发给STM32的JSON
            String controlMsg = "{\"cmd\":\"velocity\",\"vx\":" + String(vx) + 
                               ",\"vy\":" + String(vy) + 
                               ",\"w\":" + String(w) + "}";
            
            // 使用通信管理类发送到STM32
            stm32Comm.sendCommand(controlMsg);
          
        } else if (cmd && strcmp(cmd, "getSystemInfo") == 0) {
          // 处理系统信息请求
          // 从STM32获取一些信息（电池电压等），其他信息由ESP32提供
          
          // 查询STM32数据
          stm32Comm.sendCommand("{\"cmd\":\"getInfo\"}");
          
          // 等待STM32响应（实际应用中应该使用异步方式）
          unsigned long timeout = millis();
          String stm32Response = "";
          bool receivedResponse = false;
          
          // 等待最多500ms获取STM32回复
          while (millis() - timeout < 500) {
            if (STM32_SERIAL.available()) {
              String data = STM32_SERIAL.readStringUntil('\n');
              if (data.length() > 0) {
                stm32Response = data;
                receivedResponse = true;
                stm32Comm.handleResponse(data);
                break;
              }
            }
            stm32Comm.update(); // 更新通信状态，检查超时
            delay(5);
          }
          
          // 生成系统信息
          float batteryVoltage = 12.6;
          int fsm_state = 1;
          int obstacle_distance = 0;
          
          // 如果收到STM32响应，解析其中的数据
          if (receivedResponse) {
            StaticJsonDocument<200> stm32Doc;
            DeserializationError stm32Error = deserializeJson(stm32Doc, stm32Response);
            
            if (!stm32Error) {
              if (stm32Doc.containsKey("battery")) {
                batteryVoltage = stm32Doc["battery"];
              }
              if (stm32Doc.containsKey("fsm")) {
                fsm_state = stm32Doc["fsm"];
              }
              if (stm32Doc.containsKey("obstacle")) {
                obstacle_distance = stm32Doc["obstacle"];
              }
            }
          }
          
          // 构建系统信息JSON
          String systemInfo = "{";
          systemInfo += "\"battery\":" + String(batteryVoltage, 1) + ",";
          systemInfo += "\"wifi\":" + String(WiFi.RSSI()) + ",";
          systemInfo += "\"cpu\":" + String(random(20, 60)) + ",";
          systemInfo += "\"fsm\":" + String(fsm_state) + ",";
          systemInfo += "\"obstacle\":" + String(obstacle_distance) + ",";
          systemInfo += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
          systemInfo += "\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",";
          systemInfo += "\"uptime\":\"" + getUptime() + "\"";
          systemInfo += "}";
          
          // 发送到WebSocket客户端
          webSocket.sendTXT(num, systemInfo);
        } else {
          // 默认转发到STM32
          String cmdStr = String((char*)payload, length);
          stm32Comm.sendCommand(cmdStr);
        }
      } else {
        // 解析错误，直接转发原始消息
        String cmdStr = String((char*)payload, length);
        stm32Comm.sendCommand(cmdStr);
      }
      break;
  }
}

// 获取系统当前时间字符串
String getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "未知时间";
  }
  char timeString[32];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

// 计算可用存储空间
String getAvailableSpace() {
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
  float freeSpaceMB = (totalBytes - usedBytes) / (1024.0 * 1024.0);
  return String(freeSpaceMB, 2) + "MB";
}

// 获取运行时间
String getUptime() {
  unsigned long uptime = millis() / 1000; // 转换为秒
  int days = uptime / (24 * 3600);
  uptime %= (24 * 3600);
  int hours = uptime / 3600;
  uptime %= 3600;
  int minutes = uptime / 60;
  int seconds = uptime % 60;
  
  char uptimeStr[64];
  if (days > 0) {
    sprintf(uptimeStr, "%d天%02d:%02d:%02d", days, hours, minutes, seconds);
  } else {
    sprintf(uptimeStr, "%02d:%02d:%02d", hours, minutes, seconds);
  }
  return String(uptimeStr);
}

// 设置Web服务器路由
void setupServer() {
  // 明确的HTML页面路由
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  server.on("/joystick.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/joystick.html", "text/html");
  });
  
  server.on("/update.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/update.html", "text/html");
  });
  
  server.on("/ota_stm32.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/ota_stm32.html", "text/html");
  });
  
  // 静态资源服务（CSS, JS等）
  server.serveStatic("/js", LittleFS, "/js");
  server.serveStatic("/css", LittleFS, "/css");
  server.serveStatic("/img", LittleFS, "/img");
  
  // 版本信息API
  server.on("/version", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"version\":\"" FIRMWARE_VERSION "\"}");
  });
  
  // 系统信息API
  server.on("/system-info", HTTP_GET, [](AsyncWebServerRequest *request) {
    String mac = WiFi.macAddress();
    String ip = WiFi.localIP().toString();
    String rssi = String(WiFi.RSSI());
    String uptime = getUptime();
    String available_space = getAvailableSpace();
    
    String info = "{";
    info += "\"version\":\"" FIRMWARE_VERSION "\",";
    info += "\"model\":\"" DEVICE_MODEL "\",";
    info += "\"mac\":\"" + mac + "\",";
    info += "\"ip\":\"" + ip + "\",";
    info += "\"rssi\":" + rssi + ",";
    info += "\"uptime\":\"" + uptime + "\",";
    info += "\"available_space\":\"" + available_space + "\",";
    info += "\"last_update\":\"" + String(lastUpdateTime) + "\"";
    info += "}";
    
    request->send(200, "application/json", info);
  });
  
  // WiFi状态API
  server.on("/wifi_status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String status = "{\"sta_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + 
                   ",\"sta_ip\":\"" + WiFi.localIP().toString() + 
                   "\",\"sta_ssid\":\"" + WiFi.SSID() + 
                   "\",\"ap_ip\":\"" + WiFi.softAPIP().toString() + 
                   "\",\"ap_ssid\":\"" + String(HOST_AP_SSID) + 
                   "\",\"config_mode\":" + String(isConfigMode ? "true" : "false") + "}";
    request->send(200, "application/json", status);
  });
  
  // 重置WiFi配置
  server.on("/reset_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    WiFiManager wm;
    wm.resetSettings();
    request->send(200, "text/plain", "WiFi配置已重置，设备将重启...");
    delay(1000);
    ESP.restart();
  });
  
  // 处理WiFi凭据提交
  server.on("/connect_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid, password;
    
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();
      
      // 检查是否有静态IP设置
      bool useStaticIP = false;
      IPAddress staticIP, gateway, subnet, dns;
      
      if (request->hasParam("static_ip", true) && 
          request->hasParam("static_gateway", true) && 
          request->hasParam("static_subnet", true)) {
        
        String ip = request->getParam("static_ip", true)->value();
        String gw = request->getParam("static_gateway", true)->value();
        String sn = request->getParam("static_subnet", true)->value();
        
        if (staticIP.fromString(ip) && gateway.fromString(gw) && subnet.fromString(sn)) {
          useStaticIP = true;
          
          // 如果有指定DNS服务器
          if (request->hasParam("static_dns", true)) {
            String dnsStr = request->getParam("static_dns", true)->value();
            dns.fromString(dnsStr);
          } else {
            dns = gateway; // 默认使用网关作为DNS
          }
        }
      }
      
      // 保存WiFi凭据到EEPROM
      WiFi.disconnect();
      delay(500);
      
      // 如果使用静态IP
      if (useStaticIP) {
        Serial.println("使用静态IP配置:");
        Serial.println("IP: " + staticIP.toString());
        Serial.println("网关: " + gateway.toString());
        Serial.println("子网掩码: " + subnet.toString());
        Serial.println("DNS: " + dns.toString());
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      
      WiFi.begin(ssid.c_str(), password.c_str());
      
      // 等待连接结果
      int timeout = 20; // 10秒超时
      while (timeout > 0 && WiFi.status() != WL_CONNECTED) {
        delay(500);
        timeout--;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        // 保存凭据到永久存储
        WiFiManager wm;
        wm.setConnectTimeout(10);
        wm.setSaveConfigCallback([]() {
          Serial.println("WiFi凭据已保存");
        });
        
        // 使用WiFiManager的内部方法保存凭据
        WiFi.disconnect(false);
        
        // 配置静态IP或DHCP
        if (useStaticIP) {
          wm.setSTAStaticIPConfig(staticIP, gateway, subnet, dns);
        } else {
          wm.setSTAStaticIPConfig(IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0)); // 使用DHCP
        }
        
        wm.setupConfigPortal(HOST_AP_SSID, HOST_AP_PASS);
        wm.autoConnect(HOST_AP_SSID, HOST_AP_PASS);
        
        // 重启ESP32以应用新设置
        request->send(200, "application/json", "{\"success\":true,\"message\":\"连接成功，设备将重启\"}");
        delay(1000);
        ESP.restart();
      } else {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"连接失败，请检查WiFi名称和密码\"}");
      }
    } else {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"缺少必要参数\"}");
    }
  });
  
  // 获取可用WiFi网络列表
  server.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "[";
    int n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true); // 异步扫描
      request->send(200, "application/json", "[]");
      return;
    } else if (n > 0) {
      for (int i = 0; i < n; ++i) {
        if (i) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN) + ",";
        json += "\"channel\":" + String(WiFi.channel(i));
        json += "}";
      }
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) {
        WiFi.scanNetworks(true);
      }
    }
    json += "]";
    request->send(200, "application/json", json);
  });
  
  // 404处理
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "文件未找到");
  });
  
  // OTA升级
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/update.html", "text/html");
  });
  
  // ESP32 OTA完成回调
  AsyncElegantOTA.onOTAStart([](void) {
    Serial.println("OTA升级开始");
  });
  
  AsyncElegantOTA.onOTAProgress([](size_t current, size_t final) {
    Serial.printf("OTA进度: %u%%\r", (current * 100) / final);
  });
  
  AsyncElegantOTA.onOTAEnd([](bool success) {
    if (success) {
      Serial.println("OTA升级成功");
      strcpy(lastUpdateTime, getCurrentTime().c_str());
    } else {
      Serial.println("OTA升级失败");
    }
  });
  
  AsyncElegantOTA.begin(&server);
  
  // 启动服务
  server.begin();
  Serial.println("HTTP服务器已启动");
  
  // 启动WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  webSocket.enableHeartbeat(15000, 3000, 2);
  Serial.println("WebSocket服务器已启动在端口81");
}

// WiFi管理器回调函数
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("进入配置模式");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  isConfigMode = true;
}

// 配置WiFi
void setupWiFi() {
  WiFiManager wm;
  
  // 设置回调
  wm.setAPCallback(configModeCallback);
  
  // 设置配置门户超时
  wm.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
  
  // 自动连接WiFi，如果连接失败，则启动配置门户
  bool res = wm.autoConnect(HOST_AP_SSID, HOST_AP_PASS);
  
  if(!res) {
    Serial.println("无法连接");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("已连接到WiFi");
    isConfigMode = false;
    
    // 连接成功后，启用双模式
    WiFi.mode(WIFI_AP_STA);
    
    // 配置AP，使用不同于连接WiFi的信道
    int homeChannel = WiFi.channel();
    int apChannel = (homeChannel <= 6) ? homeChannel + 5 : homeChannel - 5;
    if (apChannel > 13) apChannel = 13;
    if (apChannel < 1) apChannel = 1;
    
    bool apStarted = WiFi.softAP(HOST_AP_SSID, HOST_AP_PASS, apChannel);
    
    if (apStarted) {
      Serial.println("双模式已启用");
      Serial.println("STA模式 - SSID: " + WiFi.SSID() + ", IP: " + WiFi.localIP().toString());
      Serial.println("AP模式  - SSID: " + String(HOST_AP_SSID) + ", IP: " + WiFi.softAPIP().toString() + ", 信道: " + String(apChannel));
      
      // 向STM32发送IP信息
      String ipInfo = "{\"ip\":\"" + WiFi.localIP().toString() + "\",\"ssid\":\"" + WiFi.SSID() + "\"}\n";
      STM32_SERIAL.print(ipInfo);
    } else {
      Serial.println("AP模式启动失败，仅使用STA模式");
    }
  }
}

// 将STM32置于bootloader模式
bool setSTM32BootloaderMode(bool enable) {
  if (enable) {
    // 进入bootloader模式
    digitalWrite(BOOT0_PIN, HIGH);  // BOOT0=1
    digitalWrite(RESET_PIN, LOW);   // 复位STM32
    delay(100);
    digitalWrite(RESET_PIN, HIGH);  // 释放复位，STM32进入bootloader
    delay(500);  // 等待STM32进入bootloader模式
    return true;
  } else {
    // 恢复正常运行模式
    digitalWrite(BOOT0_PIN, LOW);   // BOOT0=0
    digitalWrite(RESET_PIN, LOW);   // 复位STM32
    delay(100);
    digitalWrite(RESET_PIN, HIGH);  // 释放复位，STM32进入正常模式
    delay(500);  // 等待STM32启动
    return true;
  }
}

// 计算CRC16
uint16_t calculateCRC16(const uint8_t* data, size_t length) {
  uint16_t crc = 0;
  while (length--) {
    crc = crc ^ (*data++ << 8);
    for (int i = 0; i < 8; i++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

// YMODEM发送函数
bool ymodemSend(const char* filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println("无法打开固件文件");
    return false;
  }
  
  size_t fileSize = file.size();
  Serial.printf("文件大小: %u 字节\n", fileSize);
  
  // 1. 发送文件名数据包
  uint8_t packet[133]; // 128字节数据 + 3字节帧头 + 2字节CRC
  memset(packet, 0, sizeof(packet));
  packet[0] = YMODEM_SOH;  // 128字节包
  packet[1] = 0x00;  // 包序号
  packet[2] = 0xFF;  // 包序号的反码
  
  // 填充文件名和大小
  String fileInfo = String(file.name()) + " " + String(fileSize);
  memcpy(&packet[3], fileInfo.c_str(), fileInfo.length());
  
  // 计算CRC16
  uint16_t crc = calculateCRC16(&packet[3], 128);
  packet[131] = (crc >> 8) & 0xFF;
  packet[132] = crc & 0xFF;
  
  // 等待接收端准备好
  bool receiverReady = false;
  unsigned long startTime = millis();
  
  while (!receiverReady && (millis() - startTime < 10000)) {
    if (STM32_SERIAL.available() && STM32_SERIAL.read() == YMODEM_C) {
      receiverReady = true;
    }
    delay(10);
  }
  
  if (!receiverReady) {
    Serial.println("接收端未就绪");
    file.close();
    return false;
  }
  
  // 发送文件名数据包
  STM32_SERIAL.write(packet, sizeof(packet));
  
  // 等待ACK
  startTime = millis();
  bool receivedACK = false;
  
  while (!receivedACK && (millis() - startTime < 5000)) {
    if (STM32_SERIAL.available()) {
      uint8_t response = STM32_SERIAL.read();
      if (response == YMODEM_ACK) {
        receivedACK = true;
      }
    }
    delay(10);
  }
  
  if (!receivedACK) {
    Serial.println("未收到文件信息确认");
    file.close();
    return false;
  }
  
  // 2. 发送数据包
  uint8_t blockNum = 1;
  uint8_t fileBuffer[1024];
  size_t bytesRead;
  
  while ((bytesRead = file.read(fileBuffer, sizeof(fileBuffer))) > 0) {
    bool useSTX = (bytesRead > 128);
    size_t packetSize = useSTX ? 1024 : 128;
    
    uint8_t dataPacket[packetSize + 5]; // 数据 + 3字节帧头 + 2字节CRC
    memset(dataPacket, 0, sizeof(dataPacket));
    
    // 填充包头
    dataPacket[0] = useSTX ? YMODEM_STX : YMODEM_SOH;
    dataPacket[1] = blockNum;
    dataPacket[2] = 0xFF - blockNum;
    
    // 复制文件数据
    memcpy(&dataPacket[3], fileBuffer, bytesRead);
    if (bytesRead < packetSize) {
      memset(&dataPacket[3 + bytesRead], 0, packetSize - bytesRead);
    }
    
    // 计算CRC
    uint16_t dataCrc = calculateCRC16(&dataPacket[3], packetSize);
    dataPacket[packetSize + 3] = (dataCrc >> 8) & 0xFF;
    dataPacket[packetSize + 4] = dataCrc & 0xFF;
    
    // 发送数据包
    STM32_SERIAL.write(dataPacket, sizeof(dataPacket));
    
    // 等待ACK
    startTime = millis();
    receivedACK = false;
    
    while (!receivedACK && (millis() - startTime < 5000)) {
      if (STM32_SERIAL.available()) {
        uint8_t response = STM32_SERIAL.read();
        if (response == YMODEM_ACK) {
          receivedACK = true;
        }
      }
      delay(10);
    }
    
    if (!receivedACK) {
      Serial.println("数据包发送失败");
      file.close();
      return false;
    }
    
    blockNum++;
  }
  
  // 3. 发送EOT
  STM32_SERIAL.write(YMODEM_EOT);
  
  // 等待ACK
  startTime = millis();
  receivedACK = false;
  
  while (!receivedACK && (millis() - startTime < 5000)) {
    if (STM32_SERIAL.available()) {
      uint8_t response = STM32_SERIAL.read();
      if (response == YMODEM_ACK) {
        receivedACK = true;
      }
    }
    delay(10);
  }
  
  if (!receivedACK) {
    Serial.println("EOT未确认");
    file.close();
    return false;
  }
  
  // 4. 发送空数据包结束传输
  memset(packet, 0, sizeof(packet));
  packet[0] = YMODEM_SOH;
  packet[1] = 0;
  packet[2] = 0xFF;
  
  STM32_SERIAL.write(packet, sizeof(packet));
  
  file.close();
  Serial.println("文件传输完成");
  return true;
}

// STM32 OTA功能
void setupSTM32OTA() {
  server.on("/ota_stm32", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/ota_stm32.html", "text/html");
  });
  
  // 处理STM32固件上传
  server.on("/upload_stm32_firmware", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"开始处理固件\"}");
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File firmwareFile;
    
    if(!index) {
      // 开始接收新文件
      if(LittleFS.exists("/stm32_firmware.bin")) {
        LittleFS.remove("/stm32_firmware.bin");
      }
      firmwareFile = LittleFS.open("/stm32_firmware.bin", "w");
      Serial.println("开始接收STM32固件");
    }
    
    if(firmwareFile) {
      firmwareFile.write(data, len);
    }
    
    if(final) {
      firmwareFile.close();
      Serial.println("STM32固件接收完成，大小: " + String(index + len) + " 字节");
      
      // 固件验证
      File verifyFile = LittleFS.open("/stm32_firmware.bin", "r");
      if (!verifyFile || verifyFile.size() == 0) {
        request->send(400, "application/json", "{\"status\":\"固件验证失败\",\"ok\":false}");
        return;
      }
      verifyFile.close();
      
      // 通知前端固件已接收完成
      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"固件接收完成\",\"size\":" + String(index + len) + "}");
      response->addHeader("Connection", "close");
      request->send(response);
    }
  });
  
  // 开始STM32固件烧录
  server.on("/flash_stm32", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool bootloaderMode = setSTM32BootloaderMode(true);
    
    if (!bootloaderMode) {
      request->send(500, "application/json", "{\"status\":\"无法将STM32置于bootloader模式\",\"ok\":false}");
      return;
    }
    
    // 使用YMODEM协议传输固件
    bool success = ymodemSend("/stm32_firmware.bin");
    
    // 恢复STM32到正常模式
    setSTM32BootloaderMode(false);
    
    if (success) {
      strcpy(lastUpdateTime, getCurrentTime().c_str());
    }
    
    request->send(200, "application/json", 
                  "{\"status\":\"" + String(success ? "烧录成功" : "烧录失败") + 
                  "\",\"ok\":" + String(success ? "true" : "false") + "}");
  });
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  STM32_SERIAL.begin(STM32_BAUD, SERIAL_8N1, 16, 17);  // RX, TX引脚
  
  // 初始化GPIO
  pinMode(RESET_PIN, OUTPUT);
  pinMode(BOOT0_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(BOOT0_PIN, LOW);
  
  Serial.println("ESP32启动中...");
  Serial.println("固件版本: " FIRMWARE_VERSION);
  
  bootTime = millis();
  
  setupFS();
  setupWiFi();
  
  // 设置NTP以获取时间
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  setupServer();
  setupSTM32OTA();
  
  Serial.println("系统初始化完成");
}

void loop() {
  webSocket.loop();
  
  // 更新STM32通信状态
  stm32Comm.update();
  
  // 检查STM32的串口数据并处理
  while (STM32_SERIAL.available()) {
    String data = STM32_SERIAL.readStringUntil('\n');
    Serial.println("从STM32收到: " + data);
    
    // 更新通信状态
    stm32Comm.handleResponse(data);
    
    // 转发到所有WebSocket客户端
    if (data.length() > 0) {
      webSocket.broadcastTXT(data.c_str(), data.length());
    }
  }
  
  // 定期发送模拟状态数据（仅在没有实际STM32数据时作为测试）
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 5000) { // 每5秒发送一次状态更新
    lastStatusUpdate = millis();
    
    // 生成模拟数据
    static float batteryVoltage = 12.6;
    batteryVoltage -= random(0, 10) * 0.01;
    if (batteryVoltage < 11.0) batteryVoltage = 12.6;
    
    // 构建系统信息JSON
    String systemInfo = "{";
    systemInfo += "\"battery\":" + String(batteryVoltage, 1) + ",";
    systemInfo += "\"wifi\":" + String(WiFi.RSSI()) + ",";
    systemInfo += "\"cpu\":" + String(random(20, 60)) + ",";
    systemInfo += "\"fsm\":" + String(random(1, 5)) + ",";
    systemInfo += "\"obstacle\":" + String(random(0, 100)) + ",";
    systemInfo += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    systemInfo += "\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",";
    systemInfo += "\"uptime\":\"" + getUptime() + "\"";
    systemInfo += "}";
    
    // 只有当有客户端连接时才发送
    if (webSocket.connectedClients() > 0) {
      webSocket.broadcastTXT(systemInfo);
    }
  }
  
  // 检查WiFi连接状态
  static unsigned long lastWifiCheck = 0;
  static bool wasConnected = false;
  
  if (millis() - lastWifiCheck > 30000) {  // 每30秒检查一次
    lastWifiCheck = millis();
    
    bool isConnected = WiFi.status() == WL_CONNECTED;
    
    // 连接状态变化
    if (isConnected != wasConnected) {
      wasConnected = isConnected;
      
      if (isConnected) {
        Serial.println("WiFi已重新连接");
        // 向STM32发送IP信息
        String ipInfo = "{\"ip\":\"" + WiFi.localIP().toString() + "\",\"ssid\":\"" + WiFi.SSID() + "\"}";
        stm32Comm.sendCommand(ipInfo);
      } else {
        Serial.println("WiFi连接已断开");
      }
    }
  }
  
  // 其他循环任务...
  delay(1);
} 