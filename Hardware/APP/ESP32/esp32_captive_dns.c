#include "esp32_captive_dns.h"
#include "string.h"

// Captive DNS配置
static Captive_DNS_Config_t dns_config;

/**
 * @brief 初始化Captive DNS服务
 */
void ESP32_CaptiveDNS_Init(void)
{
    // 初始化配置
    memset(&dns_config, 0, sizeof(Captive_DNS_Config_t));
    dns_config.enabled = 0;
    strcpy(dns_config.portal_ssid, "FSM_Robot_Setup");
    strcpy(dns_config.portal_password, "12345678");
    strcpy(dns_config.redirect_url, "http://192.168.4.1/setup");
    
    // 这里可以添加实际的初始化代码
}

/**
 * @brief 启用Captive DNS服务
 */
void ESP32_CaptiveDNS_Enable(void)
{
    // 启用Captive DNS
    dns_config.enabled = 1;
    
    // 设置ESP32为AP模式
    ESP32_WiFi_SetAPMode(dns_config.portal_ssid, dns_config.portal_password);
    
    // 发送命令到ESP32启用Captive DNS
    char cmd[128];
    sprintf(cmd, "DNS_ENABLE:%s", dns_config.redirect_url);
    ESP32_WiFi_SendCommand(cmd);
}

/**
 * @brief 禁用Captive DNS服务
 */
void ESP32_CaptiveDNS_Disable(void)
{
    // 禁用Captive DNS
    dns_config.enabled = 0;
    
    // 发送命令到ESP32禁用Captive DNS
    ESP32_WiFi_SendCommand("DNS_DISABLE");
}

/**
 * @brief 设置重定向URL
 * @param url 重定向URL
 */
void ESP32_CaptiveDNS_SetRedirectURL(char *url)
{
    // 设置重定向URL
    strcpy(dns_config.redirect_url, url);
    
    // 如果已启用，更新ESP32上的设置
    if (dns_config.enabled) {
        char cmd[128];
        sprintf(cmd, "DNS_URL:%s", url);
        ESP32_WiFi_SendCommand(cmd);
    }
}

/**
 * @brief 设置Portal凭证
 * @param ssid Portal的SSID
 * @param password Portal的密码
 */
void ESP32_CaptiveDNS_SetPortalCredentials(char *ssid, char *password)
{
    // 保存凭证
    strcpy(dns_config.portal_ssid, ssid);
    strcpy(dns_config.portal_password, password);
    
    // 如果已启用，需要先禁用再重新启用
    if (dns_config.enabled) {
        ESP32_CaptiveDNS_Disable();
        ESP32_CaptiveDNS_Enable();
    }
}

/**
 * @brief 检查Captive DNS是否已启用
 * @return 1:已启用 0:未启用
 */
u8 ESP32_CaptiveDNS_IsEnabled(void)
{
    return dns_config.enabled;
} 