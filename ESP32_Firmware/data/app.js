// 机器人WebSocket控制脚本
let socket;
let isConnected = false;
const readout = document.getElementById('vx-value');
const vyValue = document.getElementById('vy-value');
const wValue = document.getElementById('w-value');
let last = 0;

// 连接WebSocket
function connectWebSocket() {
    socket = new WebSocket(`ws://${location.host}/ws`);
    
    socket.onopen = function() {
        console.log('WebSocket连接成功');
        isConnected = true;
        document.getElementById('run-status').textContent = '已连接';
    };
    
    socket.onclose = function() {
        console.log('WebSocket连接关闭');
        isConnected = false;
        document.getElementById('run-status').textContent = '已断开';
        // 尝试重新连接
        setTimeout(connectWebSocket, 2000);
    };
    
    socket.onerror = function(error) {
        console.error('WebSocket错误:', error);
        document.getElementById('run-status').textContent = '连接错误';
    };
    
    socket.onmessage = function(e) {
        console.log('收到消息:', e.data);
        try {
            const data = JSON.parse(e.data);
            // 处理从ESP32收到的数据
            if (data.battery) {
                document.getElementById('battery-voltage').textContent = data.battery + 'V';
            }
            if (data.state) {
                document.getElementById('fsm-state').textContent = data.state;
            }
            // 可以添加更多状态更新
        } catch (e) {
            console.error('解析消息错误:', e);
        }
    };
}

// 发送速度命令
function sendVelocity(vx, vy, w) {
    if (!isConnected) return;
    
    const now = Date.now();
    if (now - last < 50) return; // 限制发送频率为20Hz
    last = now;
    
    // 格式化数值，保留两位小数
    const vxFormatted = parseFloat(vx).toFixed(2);
    const vyFormatted = parseFloat(vy).toFixed(2);
    const wFormatted = parseFloat(w).toFixed(2);
    
    // 更新显示
    if (readout) readout.textContent = vxFormatted;
    if (vyValue) vyValue.textContent = vyFormatted;
    if (wValue) wValue.textContent = wFormatted;
    
    // 发送到WebSocket
    const cmd = `${vxFormatted},${vyFormatted},${wFormatted}`;
    socket.send(cmd);
    console.log('发送:', cmd);
}

// 停止机器人
function stopRobot() {
    sendVelocity(0, 0, 0);
}

// 页面加载完成后连接WebSocket
window.addEventListener('load', function() {
    connectWebSocket();
    
    // 添加急停按钮事件
    const stopBtn = document.getElementById('stop-btn');
    if (stopBtn) {
        stopBtn.addEventListener('click', stopRobot);
    }
}); 