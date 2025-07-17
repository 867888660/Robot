#ifndef __FSM_HARDWARE_H
#define __FSM_HARDWARE_H

#include "sys.h"
#include "FSM_Parser.h"

/* 硬件抽象层函数 */

// 电机控制
void FSM_HW_StopAllMotors(void);
void FSM_HW_MoveDistance(int16_t dist_cm, u8 speed);
void FSM_HW_Turn(int16_t angle_deg, u8 speed);
void FSM_HW_SetMotorRaw(int16_t m1, int16_t m2, int16_t m3, int16_t m4);

// 新增：三自由度底盘速度控制 (vx, vy, ω)
void FSM_HW_SetChassisVelocity(float vx, float vy, float w);

// LED控制
void FSM_HW_SetLED(u8 r, u8 g, u8 b, u8 count);

// OLED控制
void FSM_HW_ShowText(const char* text);
void FSM_HW_ShowEmoji(EmojiType emoji);

// 无线通信
void FSM_HW_SendNRF(const u8* data, u8 len);

// 蜂鸣器控制
void FSM_HW_Beep(u16 ms);

// 传感器读取
float FSM_HW_GetSonarDistance(void);
void FSM_HW_GetMPUData(float* pitch, float* roll, float* yaw);
const char* FSM_HW_GetIRLineStatus(void);
u8 FSM_HW_GetBatteryLevel(void);
const char* FSM_HW_GetNRFMessage(void);

// 编码器接口
void FSM_HW_InitEncoders(void);              // 初始化编码器
float FSM_HW_GetMotorRPM(u8 motor_id);       // 获取电机转速 (RPM)
float FSM_HW_GetWheelSpeed(u8 wheel_id);     // 获取轮子线速度 (m/s)
void FSM_HW_UpdateEncoders(void);            // 更新编码器数据 (内部调用)

// 系统时间
u32 FSM_HW_GetTimeMs(void);

// 系统状态
u8 FSM_HW_GetCPUUsage(void);
u8 FSM_HW_GetQueueLength(void);

#endif /* __FSM_HARDWARE_H */ 