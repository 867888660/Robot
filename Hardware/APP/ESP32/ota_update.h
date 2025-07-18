#ifndef __OTA_UPDATE_H
#define __OTA_UPDATE_H

#include "sys.h"

// OTA升级页面HTML
const char* ota_update_html = "<!DOCTYPE html>\n"
"<html lang=\"zh-CN\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>固件升级</title>\n"
"    <style>\n"
"        body {\n"
"            font-family: Arial, sans-serif;\n"
"            margin: 0;\n"
"            padding: 0;\n"
"            background-color: #f5f5f5;\n"
"            color: #333;\n"
"        }\n"
"        .container {\n"
"            max-width: 600px;\n"
"            margin: 0 auto;\n"
"            padding: 20px;\n"
"        }\n"
"        .header {\n"
"            text-align: center;\n"
"            padding: 20px 0;\n"
"            background-color: #0078d7;\n"
"            color: white;\n"
"            border-radius: 5px 5px 0 0;\n"
"            margin-bottom: 20px;\n"
"        }\n"
"        .upload-container {\n"
"            background-color: white;\n"
"            border-radius: 5px;\n"
"            padding: 20px;\n"
"            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);\n"
"        }\n"
"        .form-group {\n"
"            margin-bottom: 20px;\n"
"        }\n"
"        .form-group label {\n"
"            display: block;\n"
"            margin-bottom: 8px;\n"
"            font-weight: bold;\n"
"        }\n"
"        .form-group input[type=\"file\"] {\n"
"            width: 100%;\n"
"            padding: 10px;\n"
"            border: 1px solid #ddd;\n"
"            border-radius: 4px;\n"
"            box-sizing: border-box;\n"
"        }\n"
"        .form-group button {\n"
"            background-color: #0078d7;\n"
"            color: white;\n"
"            border: none;\n"
"            padding: 12px 20px;\n"
"            border-radius: 4px;\n"
"            cursor: pointer;\n"
"            width: 100%;\n"
"            font-size: 16px;\n"
"        }\n"
"        .form-group button:hover {\n"
"            background-color: #0056b3;\n"
"        }\n"
"        .form-group button:disabled {\n"
"            background-color: #cccccc;\n"
"            cursor: not-allowed;\n"
"        }\n"
"        .progress-container {\n"
"            margin-top: 20px;\n"
"            display: none;\n"
"        }\n"
"        .progress-bar {\n"
"            height: 20px;\n"
"            background-color: #e9ecef;\n"
"            border-radius: 4px;\n"
"            overflow: hidden;\n"
"            margin-bottom: 10px;\n"
"        }\n"
"        .progress {\n"
"            height: 100%;\n"
"            background-color: #0078d7;\n"
"            width: 0%;\n"
"            transition: width 0.3s;\n"
"        }\n"
"        .status {\n"
"            margin-top: 20px;\n"
"            padding: 15px;\n"
"            border-radius: 4px;\n"
"            display: none;\n"
"        }\n"
"        .status.success {\n"
"            background-color: #d4edda;\n"
"            color: #155724;\n"
"            display: block;\n"
"        }\n"
"        .status.error {\n"
"            background-color: #f8d7da;\n"
"            color: #721c24;\n"
"            display: block;\n"
"        }\n"
"        .status.info {\n"
"            background-color: #e2f3fd;\n"
"            color: #0c5460;\n"
"            display: block;\n"
"        }\n"
"        .system-info {\n"
"            margin-top: 20px;\n"
"            background-color: white;\n"
"            border-radius: 5px;\n"
"            padding: 20px;\n"
"            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);\n"
"        }\n"
"        .info-item {\n"
"            display: flex;\n"
"            justify-content: space-between;\n"
"            margin-bottom: 10px;\n"
"            padding-bottom: 10px;\n"
"            border-bottom: 1px solid #eee;\n"
"        }\n"
"        .info-item:last-child {\n"
"            border-bottom: none;\n"
"            margin-bottom: 0;\n"
"            padding-bottom: 0;\n"
"        }\n"
"        .footer {\n"
"            text-align: center;\n"
"            margin-top: 20px;\n"
"            color: #666;\n"
"            font-size: 12px;\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class=\"container\">\n"
"        <div class=\"header\">\n"
"            <h1>固件升级</h1>\n"
"        </div>\n"
"        <div class=\"upload-container\">\n"
"            <h2>上传新固件</h2>\n"
"            <p>请选择固件文件(.bin)进行上传。上传完成后设备将自动重启并应用新固件。</p>\n"
"            <form id=\"uploadForm\" enctype=\"multipart/form-data\">\n"
"                <div class=\"form-group\">\n"
"                    <label for=\"firmware\">固件文件:</label>\n"
"                    <input type=\"file\" id=\"firmware\" name=\"firmware\" accept=\".bin\" required>\n"
"                </div>\n"
"                <div class=\"form-group\">\n"
"                    <button type=\"submit\" id=\"uploadBtn\">上传并升级</button>\n"
"                </div>\n"
"            </form>\n"
"            <div class=\"progress-container\" id=\"progressContainer\">\n"
"                <div class=\"progress-bar\">\n"
"                    <div class=\"progress\" id=\"progressBar\"></div>\n"
"                </div>\n"
"                <div id=\"progressText\">0%</div>\n"
"            </div>\n"
"            <div id=\"statusMessage\" class=\"status\"></div>\n"
"        </div>\n"
"        <div class=\"system-info\">\n"
"            <h2>系统信息</h2>\n"
"            <div class=\"info-item\">\n"
"                <div>当前固件版本:</div>\n"
"                <div id=\"currentVersion\">v1.0.0</div>\n"
"            </div>\n"
"            <div class=\"info-item\">\n"
"                <div>设备型号:</div>\n"
"                <div id=\"deviceModel\">ESP32-WROOM-32</div>\n"
"            </div>\n"
"            <div class=\"info-item\">\n"
"                <div>可用存储空间:</div>\n"
"                <div id=\"availableSpace\">1.3MB</div>\n"
"            </div>\n"
"            <div class=\"info-item\">\n"
"                <div>MAC地址:</div>\n"
"                <div id=\"macAddress\">XX:XX:XX:XX:XX:XX</div>\n"
"            </div>\n"
"            <div class=\"info-item\">\n"
"                <div>上次升级时间:</div>\n"
"                <div id=\"lastUpdateTime\">未知</div>\n"
"            </div>\n"
"        </div>\n"
"        <div class=\"footer\">\n"
"            <p>© 2023 智能机器人系统 | 版本 1.0</p>\n"
"        </div>\n"
"    </div>\n"
"    <script>\n"
"        document.addEventListener('DOMContentLoaded', function() {\n"
"            const uploadForm = document.getElementById('uploadForm');\n"
"            const uploadBtn = document.getElementById('uploadBtn');\n"
"            const progressContainer = document.getElementById('progressContainer');\n"
"            const progressBar = document.getElementById('progressBar');\n"
"            const progressText = document.getElementById('progressText');\n"
"            const statusMessage = document.getElementById('statusMessage');\n"
"            const firmwareInput = document.getElementById('firmware');\n"
"            \n"
"            // 获取系统信息\n"
"            fetch('/system-info')\n"
"                .then(response => response.json())\n"
"                .then(data => {\n"
"                    document.getElementById('currentVersion').textContent = data.version || 'v1.0.0';\n"
"                    document.getElementById('deviceModel').textContent = data.model || 'ESP32-WROOM-32';\n"
"                    document.getElementById('availableSpace').textContent = data.available_space || '1.3MB';\n"
"                    document.getElementById('macAddress').textContent = data.mac || 'XX:XX:XX:XX:XX:XX';\n"
"                    document.getElementById('lastUpdateTime').textContent = data.last_update || '未知';\n"
"                })\n"
"                .catch(error => {\n"
"                    console.error('Error fetching system info:', error);\n"
"                });\n"
"            \n"
"            // 文件选择验证\n"
"            firmwareInput.addEventListener('change', function() {\n"
"                const file = this.files[0];\n"
"                if (file) {\n"
"                    // 验证文件类型\n"
"                    if (!file.name.endsWith('.bin')) {\n"
"                        showStatus('请选择有效的.bin固件文件', 'error');\n"
"                        this.value = '';\n"
"                        return;\n"
"                    }\n"
"                    \n"
"                    // 验证文件大小\n"
"                    const maxSize = 1.5 * 1024 * 1024; // 1.5MB\n"
"                    if (file.size > maxSize) {\n"
"                        showStatus(`文件过大，最大允许1.5MB，当前${(file.size / (1024 * 1024)).toFixed(2)}MB`, 'error');\n"
"                        this.value = '';\n"
"                        return;\n"
"                    }\n"
"                    \n"
"                    showStatus(`已选择: ${file.name} (${(file.size / 1024).toFixed(2)}KB)`, 'info');\n"
"                }\n"
"            });\n"
"            \n"
"            // 表单提交\n"
"            uploadForm.addEventListener('submit', function(e) {\n"
"                e.preventDefault();\n"
"                \n"
"                const file = firmwareInput.files[0];\n"
"                if (!file) {\n"
"                    showStatus('请选择固件文件', 'error');\n"
"                    return;\n"
"                }\n"
"                \n"
"                // 准备上传\n"
"                const formData = new FormData();\n"
"                formData.append('firmware', file);\n"
"                \n"
"                // 禁用按钮，显示进度条\n"
"                uploadBtn.disabled = true;\n"
"                progressContainer.style.display = 'block';\n"
"                showStatus('正在上传固件，请勿断开电源或关闭页面...', 'info');\n"
"                \n"
"                // 发送请求\n"
"                const xhr = new XMLHttpRequest();\n"
"                \n"
"                // 进度监听\n"
"                xhr.upload.addEventListener('progress', function(e) {\n"
"                    if (e.lengthComputable) {\n"
"                        const percentComplete = (e.loaded / e.total) * 100;\n"
"                        progressBar.style.width = percentComplete + '%';\n"
"                        progressText.textContent = percentComplete.toFixed(1) + '%';\n"
"                    }\n"
"                });\n"
"                \n"
"                // 完成处理\n"
"                xhr.addEventListener('load', function() {\n"
"                    if (xhr.status === 200) {\n"
"                        showStatus('固件上传成功！设备正在重启并应用新固件，请等待约30秒...', 'success');\n"
"                        \n"
"                        // 倒计时重定向\n"
"                        let countdown = 30;\n"
"                        const interval = setInterval(function() {\n"
"                            countdown--;\n"
"                            showStatus(`固件上传成功！设备正在重启并应用新固件，请等待${countdown}秒...`, 'success');\n"
"                            \n"
"                            if (countdown <= 0) {\n"
"                                clearInterval(interval);\n"
"                                window.location.href = '/';\n"
"                            }\n"
"                        }, 1000);\n"
"                    } else {\n"
"                        showStatus('上传失败: ' + (xhr.responseText || '未知错误'), 'error');\n"
"                        uploadBtn.disabled = false;\n"
"                    }\n"
"                });\n"
"                \n"
"                // 错误处理\n"
"                xhr.addEventListener('error', function() {\n"
"                    showStatus('网络错误，上传失败', 'error');\n"
"                    uploadBtn.disabled = false;\n"
"                });\n"
"                \n"
"                // 发送请求\n"
"                xhr.open('POST', '/ota', true);\n"
"                xhr.send(formData);\n"
"            });\n"
"            \n"
"            // 显示状态消息\n"
"            function showStatus(message, type) {\n"
"                statusMessage.textContent = message;\n"
"                statusMessage.className = 'status ' + type;\n"
"            }\n"
"        });\n"
"    </script>\n"
"</body>\n"
"</html>";

#endif /* __OTA_UPDATE_H */ 