#ifndef __FSM_CONTROL_H
#define __FSM_CONTROL_H

#include "sys.h"
#include "delay.h"
#include "usart.h"

// FSM状态定义
typedef enum {
    FSM_STATE_IDLE = 0,     // 空闲状态
    FSM_STATE_FORWARD,      // 前进状态
    FSM_STATE_BACKWARD,     // 后退状态
    FSM_STATE_LEFT,         // 左转状态
    FSM_STATE_RIGHT,        // 右转状态
    FSM_STATE_STOP,         // 停止状态
    FSM_STATE_AUTO,         // 自动模式
    FSM_STATE_MANUAL,       // 手动模式
    FSM_STATE_ERROR         // 错误状态
} FSM_State_t;

// FSM事件定义
typedef enum {
    FSM_EVENT_NONE = 0,     // 无事件
    FSM_EVENT_FORWARD,      // 前进事件
    FSM_EVENT_BACKWARD,     // 后退事件
    FSM_EVENT_LEFT,         // 左转事件
    FSM_EVENT_RIGHT,        // 右转事件
    FSM_EVENT_STOP,         // 停止事件
    FSM_EVENT_AUTO,         // 切换到自动模式
    FSM_EVENT_MANUAL,       // 切换到手动模式
    FSM_EVENT_OBSTACLE,     // 检测到障碍物
    FSM_EVENT_LINE,         // 检测到线
    FSM_EVENT_TIMEOUT       // 超时事件
} FSM_Event_t;

// 函数声明
void FSM_Init(void);
void FSM_Update(void);
void FSM_ProcessEvent(FSM_Event_t event);
FSM_State_t FSM_GetCurrentState(void);
void FSM_SetState(FSM_State_t state);
void FSM_UpdateSensors(void);
void FSM_SendHeartbeat(void);
void FSM_LoadDefaultRules(void);

#endif /* __FSM_CONTROL_H */ 