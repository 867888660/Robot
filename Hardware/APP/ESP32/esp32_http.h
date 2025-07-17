#ifndef __ESP32_HTTP_H
#define __ESP32_HTTP_H

#include "sys.h"

// HTTP请求相关函数
void ESP32_HTTP_GetConfigPage(void);
void ESP32_HTTP_SendWiFiConfig(const char* ssid, const char* password);
void ESP32_HTTP_Restart(void);
void ESP32_HTTP_ScanNetworks(void);
void ESP32_HTTP_GetStatus(void);

// 网页控制相关函数
void ESP32_HTTP_SendControlCommand(float vx, float vy, float w);
void ESP32_HTTP_StopMotors(void);
void ESP32_HTTP_GetControlPanel(void);

// OTA升级相关函数
void ESP32_HTTP_GetOTAPage(void);
void ESP32_HTTP_GetSystemInfo(void);

#endif /* __ESP32_HTTP_H */ 