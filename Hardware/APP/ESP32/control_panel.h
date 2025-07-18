#ifndef __CONTROL_PANEL_H
#define __CONTROL_PANEL_H

#include "sys.h"

// 控制面板HTML页面
const char* control_panel_html = "<!DOCTYPE html>\n"
"<html lang=\"zh-CN\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>智能机器人控制面板</title>\n"
"    <style>\n"
"        body {\n"
"            font-family: Arial, sans-serif;\n"
"            margin: 0;\n"
"            padding: 0;\n"
"            background-color: #f5f5f5;\n"
"            color: #333;\n"
"            touch-action: none;\n"
"        }\n"
"        .container {\n"
"            max-width: 800px;\n"
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
"        .control-panel {\n"
"            display: grid;\n"
"            grid-template-columns: 1fr 1fr;\n"
"            gap: 20px;\n"
"        }\n"
"        .joystick-container {\n"
"            background-color: white;\n"
"            border-radius: 5px;\n"
"            padding: 20px;\n"
"            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);\n"
"            text-align: center;\n"
"            position: relative;\n"
"        }\n"
"        .joystick {\n"
"            width: 200px;\n"
"            height: 200px;\n"
"            background-color: #f0f0f0;\n"
"            border-radius: 50%;\n"
"            margin: 0 auto;\n"
"            position: relative;\n"
"            border: 2px solid #ddd;\n"
"            touch-action: none;\n"
"        }\n"
"        .joystick-knob {\n"
"            width: 60px;\n"
"            height: 60px;\n"
"            background-color: #0078d7;\n"
"            border-radius: 50%;\n"
"            position: absolute;\n"
"            top: 50%;\n"
"            left: 50%;\n"
"            transform: translate(-50%, -50%);\n"
"            cursor: pointer;\n"
"            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);\n"
"            touch-action: none;\n"
"        }\n"
"        .status-panel {\n"
"            background-color: white;\n"
"            border-radius: 5px;\n"
"            padding: 20px;\n"
"            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);\n"
"        }\n"
"        .status-item {\n"
"            display: flex;\n"
"            justify-content: space-between;\n"
"            margin-bottom: 10px;\n"
"            padding-bottom: 10px;\n"
"            border-bottom: 1px solid #eee;\n"
"        }\n"
"        .status-item:last-child {\n"
"            border-bottom: none;\n"
"            margin-bottom: 0;\n"
"            padding-bottom: 0;\n"
"        }\n"
"        .button-panel {\n"
"            display: grid;\n"
"            grid-template-columns: repeat(2, 1fr);\n"
"            gap: 10px;\n"
"            margin-top: 20px;\n"
"        }\n"
"        .control-button {\n"
"            background-color: #0078d7;\n"
"            color: white;\n"
"            border: none;\n"
"            padding: 15px;\n"
"            border-radius: 5px;\n"
"            cursor: pointer;\n"
"            font-size: 16px;\n"
"            text-align: center;\n"
"            transition: background-color 0.2s;\n"
"        }\n"
"        .control-button:hover {\n"
"            background-color: #0056b3;\n"
"        }\n"
"        .control-button.stop {\n"
"            background-color: #dc3545;\n"
"            grid-column: span 2;\n"
"        }\n"
"        .control-button.stop:hover {\n"
"            background-color: #c82333;\n"
"        }\n"
"        .velocity-display {\n"
"            margin-top: 20px;\n"
"            text-align: center;\n"
"            font-size: 14px;\n"
"        }\n"
"        .velocity-value {\n"
"            font-weight: bold;\n"
"            color: #0078d7;\n"
"        }\n"
"        .footer {\n"
"            text-align: center;\n"
"            margin-top: 20px;\n"
"            color: #666;\n"
"            font-size: 12px;\n"
"        }\n"
"        @media (max-width: 768px) {\n"
"            .control-panel {\n"
"                grid-template-columns: 1fr;\n"
"            }\n"
"            .joystick {\n"
"                width: 150px;\n"
"                height: 150px;\n"
"            }\n"
"            .joystick-knob {\n"
"                width: 45px;\n"
"                height: 45px;\n"
"            }\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class=\"container\">\n"
"        <div class=\"header\">\n"
"            <h1>智能机器人控制面板</h1>\n"
"        </div>\n"
"        <div class=\"control-panel\">\n"
"            <div class=\"joystick-container\">\n"
"                <h2>移动控制</h2>\n"
"                <div id=\"joystick\" class=\"joystick\">\n"
"                    <div id=\"knob\" class=\"joystick-knob\"></div>\n"
"                </div>\n"
"                <div class=\"velocity-display\">\n"
"                    <div>前进速度: <span id=\"vx-value\" class=\"velocity-value\">0.00</span> m/s</div>\n"
"                    <div>横向速度: <span id=\"vy-value\" class=\"velocity-value\">0.00</span> m/s</div>\n"
"                    <div>旋转速度: <span id=\"w-value\" class=\"velocity-value\">0.00</span> rad/s</div>\n"
"                </div>\n"
"                <div class=\"button-panel\">\n"
"                    <button id=\"forward-btn\" class=\"control-button\">前进</button>\n"
"                    <button id=\"backward-btn\" class=\"control-button\">后退</button>\n"
"                    <button id=\"left-btn\" class=\"control-button\">左转</button>\n"
"                    <button id=\"right-btn\" class=\"control-button\">右转</button>\n"
"                    <button id=\"stop-btn\" class=\"control-button stop\">急停</button>\n"
"                </div>\n"
"            </div>\n"
"            <div class=\"status-panel\">\n"
"                <h2>状态信息</h2>\n"
"                <div class=\"status-item\">\n"
"                    <div>电池电压:</div>\n"
"                    <div id=\"battery-voltage\">12.6V</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>WiFi信号:</div>\n"
"                    <div id=\"wifi-signal\">-65dBm</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>CPU使用率:</div>\n"
"                    <div id=\"cpu-usage\">35%</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>运行状态:</div>\n"
"                    <div id=\"run-status\">正常</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>当前状态机状态:</div>\n"
"                    <div id=\"fsm-state\">1</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>障碍物距离:</div>\n"
"                    <div id=\"obstacle-distance\">无障碍物</div>\n"
"                </div>\n"
"                <h3>系统信息</h3>\n"
"                <div class=\"status-item\">\n"
"                    <div>IP地址:</div>\n"
"                    <div id=\"ip-address\">192.168.1.100</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>固件版本:</div>\n"
"                    <div id=\"firmware-version\">v1.0</div>\n"
"                </div>\n"
"                <div class=\"status-item\">\n"
"                    <div>运行时间:</div>\n"
"                    <div id=\"uptime\">01:23:45</div>\n"
"                </div>\n"
"            </div>\n"
"        </div>\n"
"        <div class=\"footer\">\n"
"            <p>© 2023 智能机器人系统 | 版本 1.0</p>\n"
"        </div>\n"
"    </div>\n"
"    <script>\n"
"        // 虚拟摇杆控制\n"
"        const joystick = document.getElementById('joystick');\n"
"        const knob = document.getElementById('knob');\n"
"        const vxValue = document.getElementById('vx-value');\n"
"        const vyValue = document.getElementById('vy-value');\n"
"        const wValue = document.getElementById('w-value');\n"
"        \n"
"        // 状态元素\n"
"        const batteryVoltage = document.getElementById('battery-voltage');\n"
"        const wifiSignal = document.getElementById('wifi-signal');\n"
"        const cpuUsage = document.getElementById('cpu-usage');\n"
"        const runStatus = document.getElementById('run-status');\n"
"        const fsmState = document.getElementById('fsm-state');\n"
"        const obstacleDistance = document.getElementById('obstacle-distance');\n"
"        const ipAddress = document.getElementById('ip-address');\n"
"        const firmwareVersion = document.getElementById('firmware-version');\n"
"        const uptime = document.getElementById('uptime');\n"
"        \n"
"        // 按钮元素\n"
"        const forwardBtn = document.getElementById('forward-btn');\n"
"        const backwardBtn = document.getElementById('backward-btn');\n"
"        const leftBtn = document.getElementById('left-btn');\n"
"        const rightBtn = document.getElementById('right-btn');\n"
"        const stopBtn = document.getElementById('stop-btn');\n"
"        \n"
"        // 摇杆参数\n"
"        let isDragging = false;\n"
"        let maxDistance = joystick.clientWidth / 2 - knob.clientWidth / 2;\n"
"        let centerX = joystick.clientWidth / 2;\n"
"        let centerY = joystick.clientHeight / 2;\n"
"        let currentX = centerX;\n"
"        let currentY = centerY;\n"
"        \n"
"        // 速度参数\n"
"        let vx = 0;\n"
"        let vy = 0;\n"
"        let w = 0;\n"
"        const maxSpeed = 0.5; // 最大线速度 m/s\n"
"        const maxAngular = 1.0; // 最大角速度 rad/s\n"
"        \n"
"        // 摇杆事件处理\n"
"        function handleStart(e) {\n"
"            isDragging = true;\n"
"            e.preventDefault();\n"
"        }\n"
"        \n"
"        function handleMove(e) {\n"
"            if (!isDragging) return;\n"
"            e.preventDefault();\n"
"            \n"
"            // 获取触摸或鼠标位置\n"
"            let clientX, clientY;\n"
"            if (e.type === 'touchmove') {\n"
"                clientX = e.touches[0].clientX;\n"
"                clientY = e.touches[0].clientY;\n"
"            } else {\n"
"                clientX = e.clientX;\n"
"                clientY = e.clientY;\n"
"            }\n"
"            \n"
"            // 计算相对于摇杆中心的位置\n"
"            const rect = joystick.getBoundingClientRect();\n"
"            const offsetX = clientX - rect.left - centerX;\n"
"            const offsetY = clientY - rect.top - centerY;\n"
"            \n"
"            // 计算距离和角度\n"
"            const distance = Math.min(maxDistance, Math.sqrt(offsetX * offsetX + offsetY * offsetY));\n"
"            const angle = Math.atan2(offsetY, offsetX);\n"
"            \n"
"            // 计算摇杆位置\n"
"            currentX = centerX + distance * Math.cos(angle);\n"
"            currentY = centerY + distance * Math.sin(angle);\n"
"            \n"
"            // 更新摇杆位置\n"
"            knob.style.left = currentX + 'px';\n"
"            knob.style.top = currentY + 'px';\n"
"            \n"
"            // 计算速度\n"
"            const normalizedDistance = distance / maxDistance;\n"
"            vx = -normalizedDistance * Math.sin(angle) * maxSpeed; // 前后方向\n"
"            vy = normalizedDistance * Math.cos(angle) * maxSpeed; // 左右方向\n"
"            \n"
"            // 旋转速度 (简化为左右方向的分量)\n"
"            w = normalizedDistance * Math.cos(angle) * maxAngular;\n"
"            \n"
"            // 更新显示\n"
"            updateVelocityDisplay();\n"
"            \n"
"            // 发送控制命令\n"
"            sendVelocityCommand();\n"
"        }\n"
"        \n"
"        function handleEnd(e) {\n"
"            if (!isDragging) return;\n"
"            isDragging = false;\n"
"            e.preventDefault();\n"
"            \n"
"            // 摇杆回中\n"
"            knob.style.left = centerX + 'px';\n"
"            knob.style.top = centerY + 'px';\n"
"            currentX = centerX;\n"
"            currentY = centerY;\n"
"            \n"
"            // 速度归零\n"
"            vx = 0;\n"
"            vy = 0;\n"
"            w = 0;\n"
"            \n"
"            // 更新显示\n"
"            updateVelocityDisplay();\n"
"            \n"
"            // 发送停止命令\n"
"            sendVelocityCommand();\n"
"        }\n"
"        \n"
"        // 更新速度显示\n"
"        function updateVelocityDisplay() {\n"
"            vxValue.textContent = vx.toFixed(2);\n"
"            vyValue.textContent = vy.toFixed(2);\n"
"            wValue.textContent = w.toFixed(2);\n"
"        }\n"
"        \n"
"        // 发送速度命令\n"
"        function sendVelocityCommand() {\n"
"            fetch(`/cmd?vx=${vx.toFixed(2)}&vy=${vy.toFixed(2)}&w=${w.toFixed(2)}`);\n"
"        }\n"
"        \n"
"        // 按钮控制\n"
"        forwardBtn.addEventListener('mousedown', () => { vx = maxSpeed; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        forwardBtn.addEventListener('mouseup', () => { vx = 0; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        \n"
"        backwardBtn.addEventListener('mousedown', () => { vx = -maxSpeed; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        backwardBtn.addEventListener('mouseup', () => { vx = 0; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        \n"
"        leftBtn.addEventListener('mousedown', () => { w = maxAngular; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        leftBtn.addEventListener('mouseup', () => { w = 0; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        \n"
"        rightBtn.addEventListener('mousedown', () => { w = -maxAngular; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        rightBtn.addEventListener('mouseup', () => { w = 0; updateVelocityDisplay(); sendVelocityCommand(); });\n"
"        \n"
"        stopBtn.addEventListener('click', () => {\n"
"            vx = 0;\n"
"            vy = 0;\n"
"            w = 0;\n"
"            updateVelocityDisplay();\n"
"            \n"
"            // 发送急停命令\n"
"            fetch('/stop', { method: 'POST' });\n"
"        });\n"
"        \n"
"        // 添加触摸和鼠标事件监听\n"
"        knob.addEventListener('mousedown', handleStart);\n"
"        document.addEventListener('mousemove', handleMove);\n"
"        document.addEventListener('mouseup', handleEnd);\n"
"        \n"
"        knob.addEventListener('touchstart', handleStart);\n"
"        document.addEventListener('touchmove', handleMove);\n"
"        document.addEventListener('touchend', handleEnd);\n"
"        \n"
"        // 定期获取状态信息\n"
"        function updateStatus() {\n"
"            fetch('/status')\n"
"                .then(response => response.json())\n"
"                .then(data => {\n"
"                    // 更新状态显示\n"
"                    if (data.battery) batteryVoltage.textContent = data.battery + 'V';\n"
"                    if (data.wifi_rssi) wifiSignal.textContent = data.wifi_rssi + 'dBm';\n"
"                    if (data.cpu) cpuUsage.textContent = data.cpu + '%';\n"
"                    if (data.state) fsmState.textContent = data.state;\n"
"                    if (data.status) runStatus.textContent = data.status;\n"
"                    if (data.distance) {\n"
"                        obstacleDistance.textContent = data.distance + 'cm';\n"
"                    } else {\n"
"                        obstacleDistance.textContent = '无障碍物';\n"
"                    }\n"
"                    if (data.ip) ipAddress.textContent = data.ip;\n"
"                    if (data.version) firmwareVersion.textContent = data.version;\n"
"                    if (data.uptime) uptime.textContent = formatUptime(data.uptime);\n"
"                })\n"
"                .catch(error => console.error('Error fetching status:', error));\n"
"        }\n"
"        \n"
"        // 格式化运行时间\n"
"        function formatUptime(seconds) {\n"
"            const hours = Math.floor(seconds / 3600);\n"
"            const minutes = Math.floor((seconds % 3600) / 60);\n"
"            const secs = seconds % 60;\n"
"            return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;\n"
"        }\n"
"        \n"
"        // 页面加载时初始化\n"
"        window.addEventListener('load', () => {\n"
"            // 初始化摇杆位置\n"
"            centerX = joystick.clientWidth / 2;\n"
"            centerY = joystick.clientHeight / 2;\n"
"            maxDistance = joystick.clientWidth / 2 - knob.clientWidth / 2;\n"
"            \n"
"            // 设置摇杆初始位置\n"
"            knob.style.left = centerX + 'px';\n"
"            knob.style.top = centerY + 'px';\n"
"            \n"
"            // 初始获取状态\n"
"            updateStatus();\n"
"            \n"
"            // 定期更新状态\n"
"            setInterval(updateStatus, 1000);\n"
"        });\n"
"        \n"
"        // 窗口大小改变时重新计算摇杆参数\n"
"        window.addEventListener('resize', () => {\n"
"            centerX = joystick.clientWidth / 2;\n"
"            centerY = joystick.clientHeight / 2;\n"
"            maxDistance = joystick.clientWidth / 2 - knob.clientWidth / 2;\n"
"            \n"
"            knob.style.left = centerX + 'px';\n"
"            knob.style.top = centerY + 'px';\n"
"        });\n"
"    </script>\n"
"</body>\n"
"</html>";

#endif /* __CONTROL_PANEL_H */ 