#include "esp32_http.h"
#include "string.h"

// HTTP响应回调函数
static void (*http_callback)(HTTP_Response_t *) = NULL;

/**
 * @brief 初始化HTTP模块
 */
void ESP32_HTTP_Init(void)
{
    // 这里可以添加HTTP模块初始化代码
}

/**
 * @brief 发送HTTP请求
 * @param method 请求方法
 * @param url 请求URL
 * @param headers 请求头
 * @param body 请求体
 * @return 0:失败 1:成功
 */
u8 ESP32_HTTP_SendRequest(HTTP_Method_t method, char *url, char *headers, char *body)
{
    // 检查WiFi连接状态
    if (!ESP32_WiFi_IsConnected()) {
        return 0;
    }
    
    // 构造HTTP请求命令
    char cmd[32];
    sprintf(cmd, "HTTP:%d", (int)method);
    ESP32_WiFi_SendCommand(cmd);
    
    // 发送URL
    ESP32_WiFi_SendCommand(url);
    
    // 发送头部
    if (headers != NULL) {
        ESP32_WiFi_SendCommand(headers);
    } else {
        ESP32_WiFi_SendCommand("NULL");
    }
    
    // 发送请求体
    if (body != NULL) {
        ESP32_WiFi_SendCommand(body);
    } else {
        ESP32_WiFi_SendCommand("NULL");
    }
    
    return 1;
}

/**
 * @brief 获取HTTP状态
 * @return 0:空闲 1:忙
 */
u8 ESP32_HTTP_GetStatus(void)
{
    // 这里可以添加获取HTTP状态的代码
    return 0;
}

/**
 * @brief 处理HTTP响应
 * @param response_data 响应数据
 */
void ESP32_HTTP_ProcessResponse(char *response_data)
{
    // 解析HTTP响应
    HTTP_Response_t response;
    memset(&response, 0, sizeof(HTTP_Response_t));
    
    // 解析状态码
    char *status_start = strstr(response_data, "STATUS:");
    if (status_start) {
        status_start += 7; // 跳过"STATUS:"
        response.status_code = atoi(status_start);
    }
    
    // 解析内容类型
    char *content_type_start = strstr(response_data, "CONTENT-TYPE:");
    if (content_type_start) {
        content_type_start += 13; // 跳过"CONTENT-TYPE:"
        char *content_type_end = strchr(content_type_start, '\r');
        if (content_type_end) {
            u16 len = content_type_end - content_type_start;
            if (len < sizeof(response.content_type)) {
                strncpy(response.content_type, content_type_start, len);
                response.content_type[len] = '\0';
            }
        }
    }
    
    // 解析内容长度
    char *content_length_start = strstr(response_data, "CONTENT-LENGTH:");
    if (content_length_start) {
        content_length_start += 15; // 跳过"CONTENT-LENGTH:"
        response.content_length = atoi(content_length_start);
    }
    
    // 解析响应体
    char *body_start = strstr(response_data, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // 跳过"\r\n\r\n"
        response.body = body_start;
    }
    
    // 调用回调函数
    if (http_callback != NULL) {
        http_callback(&response);
    }
}

/**
 * @brief 设置HTTP响应回调函数
 * @param callback 回调函数
 */
void ESP32_HTTP_SetCallback(void (*callback)(HTTP_Response_t *))
{
    http_callback = callback;
} 