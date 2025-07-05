#ifndef __FSM_CONTROL_H
#define __FSM_CONTROL_H

#include "sys.h"

/* 状态机控制接口函数 */

// 初始化状态机
void FSM_Init(void);

// 加载默认规则
void FSM_LoadDefaultRules(void);

// 解析JSON规则
u8 FSM_ParseJSON(const char* json);

// 更新传感器数据
void FSM_UpdateSensors(void);

// 更新状态机
void FSM_Update(void);

// 获取当前状态
u8 FSM_GetCurrentState(void);

// 设置当前状态
void FSM_SetCurrentState(u8 state);

// 发送心跳报告
void FSM_SendHeartbeat(void);

// 发送警报
void FSM_SendAlert(const char* alertCode, const char* detail);

#endif /* __FSM_CONTROL_H */ 