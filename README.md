# STM32机器人状态机框架

基于JSON规则的STM32机器人控制系统，实现了可配置的状态机逻辑，支持多种传感器和动作执行。

## 项目结构

```
robot/
  ├── HARDWARE/
  │   ├── FSM/                   # 状态机硬件抽象层
  │   │   ├── fsm_parser.h       # 状态机解析器头文件
  │   │   ├── fsm_parser.c       # 状态机解析器实现
  │   │   ├── fsm_hardware.h     # 硬件抽象层头文件
  │   │   └── fsm_hardware.c     # 硬件抽象层实现
  │   └── ...                    # 其他硬件模块
  ├── APP/
  │   └── Control/
  │       ├── fsm_control.h      # 状态机控制接口头文件
  │       └── fsm_control.c      # 状态机控制接口实现
  └── USER/
      ├── main.c                 # 主程序入口
      └── ...                    # 其他用户文件
```

## 功能特点

- **基于JSON的规则配置**：通过JSON格式定义状态转换规则、条件和动作
- **优先级抢占机制**：高优先级规则可以抢占低优先级规则
- **多传感器支持**：支持超声波、MPU6050、红外线等多种传感器
- **多种动作执行**：支持电机控制、LED控制、OLED显示等
- **状态报告机制**：定期发送心跳、事件和动作报告
- **异步动作支持**：支持同步和异步动作执行

## 状态机规则格式

状态机规则使用JSON格式定义，包含以下主要字段：

```json
{
  "script_ver": "YYYY-MM-DD-tag",
  "transitions": [
    {
      "id": "RULE_ID",
      "priority": 100,
      "state_in": [1, 3],
      "when": {"sensor":"SNR", "cmp":"<", "value":10},
      "actions": [{"cmd":0}, {"cmd":8, "name":"SAD"}],
      "state_out": 2,
      "note": "说明文字"
    }
  ]
}
```

### 字段说明

- **id**: 规则唯一标识符
- **priority**: 优先级（0-255，越大越优先）
- **state_in**: 触发前所处状态数组
- **when**: 条件表达式
- **actions**: 动作数组
- **state_out**: 执行后的新状态
- **note**: 备注（可选）

### 条件表达式

条件表达式支持原子条件和组合条件：

#### 原子条件
```json
{"sensor":"SNR", "cmp":"<", "value":10}
```

#### 组合条件
```json
{"op":"AND", "conds":[
  {"sensor":"SNR", "cmp":"<", "value":10},
  {"sensor":"BAT", "cmp":">", "value":20}
]}
```

### 动作命令

| cmd | 名称 | 主要参数 | 说明 |
|-----|------|---------|------|
| 0 | STOP_ALL | - | 停止全部电机 |
| 1 | MOVE_CM | dist, spd | 前/后移动 |
| 2 | TURN_DEG | angle, spd | 原地转向 |
| 3 | SET_MOTOR_RAW | m1..m4 | 直接PWM控制 |
| 4 | SET_LED | r, g, b, count | WS2812B设色 |
| 5 | OLED_TEXT | text | OLED显示文本 |
| 6 | NRF_SEND | data | 2.4GHz发送数据 |
| 7 | BEEP | ms | 蜂鸣器控制 |
| 8 | OLED_EMOJI | name | OLED显示表情 |

## 状态报告格式

### 心跳报告 (HB)
```json
{
  "type": "HB",
  "seq": 6152,
  "tick": 23145678,
  "state": 1,
  "battery": 86,
  "cpu": 24,
  "queue": 2
}
```

### 事件报告 (EV)
```json
{
  "type": "EV",
  "seq": 6153,
  "event_id": "STOP_BY_SONAR",
  "state_in": 1,
  "state_out": 2,
  "cause": {
    "SNR": 8.6
  },
  "ts": 23145702
}
```

### 动作报告 (AC)
```json
{
  "type": "AC",
  "seq": 6154,
  "seq_id": 0x1A3F,
  "cmd": 0,
  "result": "OK",
  "t_start": 23145703,
  "t_end": 23146135,
  "delta": {
    "wheel_pwm_ms": 430,
    "dist_cm": 0.0
  }
}
```

### 警报 (AL)
```json
{
  "type": "AL",
  "seq": 6155,
  "alert_code": "LOW_BATT",
  "detail": {"battery": 14},
  "ts": 23146200
}
```

## 使用方法

### 1. 硬件集成

1. 将`fsm_parser.h/c`和`fsm_hardware.h/c`文件添加到HARDWARE/FSM目录
2. 将`fsm_control.h/c`文件添加到APP/Control目录
3. 修改`fsm_hardware.c`中的硬件接口函数，适配您的具体硬件

### 2. 软件集成

在`main.c`中添加以下代码：

```c
#include "fsm_control.h"

int main(void)
{
    // 初始化系统
    SystemInit();
    
    // 初始化状态机
    FSM_Init();
    
    // 加载默认规则（可选）
    FSM_LoadDefaultRules();
    
    // 主循环
    while(1)
    {
        // 更新传感器数据
        FSM_UpdateSensors();
        
        // 运行状态机
        FSM_Update();
        
        // 延时
        delay_ms(10);
    }
}
```

### 3. 发送规则

通过串口向STM32发送JSON格式的规则：

```
{
  "script_ver":"2025-07-05-alpha3",
  "transitions":[
    { "id":"STOP_BY_SONAR","priority":100,"state_in":[1,3],
      "when":{"sensor":"SNR","cmp":"<","value":10},
      "actions":[{"cmd":0},{"cmd":8,"name":"SAD"}],
      "state_out":2,"note":"急停+显示悲伤表情" }
  ]
}
```

## 注意事项

1. **内存管理**：状态机使用动态内存分配，请确保有足够的堆空间
2. **JSON解析**：建议使用cJSON库进行完整解析
3. **传感器校准**：使用前请校准各传感器
4. **优先级设置**：安全相关规则应设置高优先级（100以上）
5. **异步动作**：异步动作不会阻塞状态机，但可能导致动作重叠

## 示例应用

### 避障小车

```json
{
  "transitions":[
    { "id":"STOP_BY_SONAR","priority":100,"state_in":[1],
      "when":{"sensor":"SNR","cmp":"<","value":15},
      "actions":[{"cmd":0},{"cmd":2,"angle":-90,"spd":150},{"cmd":1,"dist":20,"spd":150}],
      "state_out":1 },
    { "id":"NORMAL_MOVE","priority":10,"state_in":[1],
      "when":{"op":"TRUE"},
      "actions":[{"cmd":1,"dist":5,"spd":150,"async":true}],
      "state_out":1 }
  ]
}
```

### 巡线小车

```json
{
  "transitions":[
    { "id":"FOLLOW_LINE","priority":80,"state_in":[1],
      "when":{"sensor":"IR_LINE","cmp":"==","value":"BLACK"},
      "actions":[{"cmd":1,"dist":2,"spd":120}],
      "state_out":1 },
    { "id":"FIND_LINE","priority":50,"state_in":[1],
      "when":{"sensor":"IR_LINE","cmp":"==","value":"WHITE"},
      "actions":[{"cmd":2,"angle":10,"spd":100},{"cmd":1,"dist":1,"spd":100}],
      "state_out":1 }
  ]
}
```

## 扩展开发

### 添加新传感器

1. 在`fsm_parser.h`中的`SensorType`枚举中添加新传感器类型
2. 在`fsm_hardware.c`中实现传感器读取函数
3. 在`fsm_control.c`的`FSM_UpdateSensors()`中调用读取函数

### 添加新动作

1. 在`fsm_parser.h`中的`CommandType`枚举中添加新动作类型
2. 在`fsm_parser.c`中实现动作执行函数
3. 在`FSM_ExecuteAction()`中添加对应的case分支

## 许可证

MIT License 