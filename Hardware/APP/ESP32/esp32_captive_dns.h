#ifndef __ESP32_CAPTIVE_DNS_H
#define __ESP32_CAPTIVE_DNS_H

#include "sys.h"
#include "esp32_wifi.h"

// Captive Portal配置
typedef struct {
    u8 enabled;              // 是否启用
    char portal_ssid[32];    // Portal的SSID
    char portal_password[64];// Portal的密码
    char redirect_url[64];   // 重定向URL
} Captive_DNS_Config_t;

// 函数声明
void ESP32_CaptiveDNS_Init(void);
void ESP32_CaptiveDNS_Enable(void);
void ESP32_CaptiveDNS_Disable(void);
void ESP32_CaptiveDNS_SetRedirectURL(char *url);
void ESP32_CaptiveDNS_SetPortalCredentials(char *ssid, char *password);
u8 ESP32_CaptiveDNS_IsEnabled(void);

#endif /* __ESP32_CAPTIVE_DNS_H */ 