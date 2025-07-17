#ifndef __ESP32_WIFI_H
#define __ESP32_WIFI_H

#include "sys.h"

// ESP32 WiFi状态定义
typedef enum {
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_AP_MODE
} ESP32_WiFi_Status_t;

// ESP32 WiFi配置结构体
typedef struct {
    ESP32_WiFi_Status_t status;    // WiFi连接状态
    char ssid[32];                 // WiFi SSID
    char password[64];             // WiFi密码
    char ip_address[16];           // IP地址
    s8 rssi;                       // 信号强度
} ESP32_WiFi_t;

// 全局WiFi状态变量
extern ESP32_WiFi_t g_esp32_wifi;

// 函数声明
void ESP32_WiFi_Init(u32 baud_rate);
void ESP32_WiFi_Process(void);
void ESP32_WiFi_SendCommand(char *cmd);
u8 ESP32_WiFi_IsConnected(void);
void ESP32_WiFi_SetAPMode(char *ssid, char *password);
void ESP32_WiFi_Connect(char *ssid, char *password);
void ESP32_WiFi_Disconnect(void);

#endif /* __ESP32_WIFI_H */ 