#ifndef __CHASSIS_SOLVER_H
#define __CHASSIS_SOLVER_H

#include "sys.h"

/* 机器人底盘三自由度解算器 (vx, vy, ω → 4 轮目标 PWM) */

// 设置期望底盘速度 (单位: m/s, rad/s)
void Chassis_SetVelocity(float vx, float vy, float w);

// 闭环循环 (建议 1 kHz) — 读取编码器, PID 调速, 更新 PWM
void Chassis_Loop_Update(void);

#endif /* __CHASSIS_SOLVER_H */ 