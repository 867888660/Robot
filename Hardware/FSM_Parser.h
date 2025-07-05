#ifndef __FSM_PARSER_H
#define __FSM_PARSER_H

#include "sys.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* 状态定义 */
#define STATE_INIT            0
#define STATE_NORMAL          1
#define STATE_EMERGENCY_STOP  2
#define STATE_REMOTE_CTRL     3
#define STATE_AUTO_DOCK       4

/* 传感器类型枚举 */
typedef enum {
    SENSOR_SNR = 0,      // 超声波
    SENSOR_MPU_ROLL,     // 横滚角
    SENSOR_MPU_PITCH,    // 俯仰角
    SENSOR_MPU_YAW,      // 航向角
    SENSOR_IR_LINE,      // 红外线传感器
    SENSOR_BAT,          // 电池电量
    SENSOR_NRF_MSG,      // 无线消息
    SENSOR_MAX           // 传感器数量
} SensorType;

/* 比较操作符 */
typedef enum {
    CMP_EQ = 0,  // ==
    CMP_NE,      // !=
    CMP_LT,      // <
    CMP_GT,      // >
    CMP_LE,      // <=
    CMP_GE       // >=
} CompareOp;

/* 逻辑操作符 */
typedef enum {
    LOGIC_AND = 0,
    LOGIC_OR,
    LOGIC_NOT,
    LOGIC_TRUE
} LogicOp;

/* 动作命令 */
typedef enum {
    CMD_STOP_ALL = 0,
    CMD_MOVE_CM,
    CMD_TURN_DEG,
    CMD_SET_MOTOR_RAW,
    CMD_SET_LED,
    CMD_OLED_TEXT,
    CMD_NRF_SEND,
    CMD_BEEP,
    CMD_OLED_EMOJI
} CommandType;

/* 表情类型 */
typedef enum {
    EMOJI_HAPPY = 0,
    EMOJI_SAD,
    EMOJI_ANGRY,
    EMOJI_SURPRISED,
    EMOJI_NEUTRAL,
    EMOJI_MAX
} EmojiType;

/* 传感器值结构体 */
typedef struct {
    union {
        float numValue;          // 数值型传感器
        char strValue[32];       // 字符串型传感器
    };
    u8 isString;                 // 是否为字符串类型
    u32 timestamp;               // 最后更新时间戳
} SensorValue;

/* 原子条件结构体 */
typedef struct {
    SensorType sensor;           // 传感器类型
    CompareOp cmp;               // 比较操作符
    union {
        float numValue;          // 数值型比较值
        char strValue[32];       // 字符串型比较值
    };
    u8 isString;                 // 是否为字符串类型
    u16 for_ms;                  // 持续时间(可选)
    u32 startTime;               // 条件开始满足的时间
} AtomicCondition;

/* 组合条件结构体(前向声明) */
typedef struct CompositeCondition CompositeCondition;

/* 条件结构体 */
typedef struct {
    u8 isAtomic;                 // 是否为原子条件
    union {
        AtomicCondition atomic;
        CompositeCondition* composite;
    };
} Condition;

/* 组合条件结构体 */
struct CompositeCondition {
    LogicOp op;                  // 逻辑操作符
    Condition* conditions;       // 子条件数组
    u8 condCount;                // 子条件数量
};

/* 动作参数结构体 */
typedef struct {
    CommandType cmd;             // 命令类型
    union {
        struct {                 // CMD_MOVE_CM
            int16_t dist;
            u8 spd;
        } move;
        
        struct {                 // CMD_TURN_DEG
            int16_t angle;
            u8 spd;
        } turn;
        
        struct {                 // CMD_SET_MOTOR_RAW
            int16_t m1;
            int16_t m2;
            int16_t m3;
            int16_t m4;
        } motor;
        
        struct {                 // CMD_SET_LED
            u8 r;
            u8 g;
            u8 b;
            u8 count;
        } led;
        
        struct {                 // CMD_OLED_TEXT
            char text[22];       // 最多21个字符+\0
        } text;
        
        struct {                 // CMD_NRF_SEND
            u8 data[32];
            u8 len;
        } nrf;
        
        struct {                 // CMD_BEEP
            u16 ms;
        } beep;
        
        struct {                 // CMD_OLED_EMOJI
            EmojiType name;
        } emoji;
    };
    u8 async;                    // 是否异步执行
    u8 completed;                // 是否已完成
} Action;

/* 转换规则结构体 */
typedef struct {
    char id[33];                 // 规则ID
    u8 priority;                 // 优先级
    u8* state_in;                // 触发前状态数组
    u8 stateInCount;             // 触发前状态数量
    Condition condition;         // 触发条件
    Action* actions;             // 动作数组
    u8 actionCount;              // 动作数量
    u8 state_out;                // 执行后状态
    char note[64];               // 备注(可选)
    u8 isExecuting;              // 是否正在执行
    u8 currentAction;            // 当前执行的动作索引
} Transition;

/* 状态机结构体 */
typedef struct {
    char script_ver[32];         // 脚本版本
    u32 timestamp;               // 时间戳
    Transition* transitions;     // 转换规则数组
    u8 transitionCount;          // 规则数量
    u8 currentState;             // 当前状态
    SensorValue sensors[SENSOR_MAX]; // 传感器值
} StateMachine;

/* 函数声明 */
void FSM_Init(void);
u8 FSM_ParseJSON(const char* json);
void FSM_Update(void);
void FSM_UpdateSensor(SensorType sensor, float value);
void FSM_UpdateSensorStr(SensorType sensor, const char* value);
u8 FSM_EvaluateCondition(const Condition* condition);
void FSM_ExecuteAction(const Action* action);
void FSM_SendHeartbeat(void);
void FSM_SendEventReport(const Transition* transition);
void FSM_SendActionReport(const Action* action, const char* result);
void FSM_SendAlert(const char* alertCode, const char* detail);

#endif /* __FSM_PARSER_H */ 