#ifndef __CHASSIS_SOLVER_H
#define __CHASSIS_SOLVER_H

#include "sys.h"

// 设置底盘速度向量 (vx, vy, w)
void Chassis_SetVelocity(float vx, float vy, float w);

// 初始化底盘控制器（包括PID控制器）
void Chassis_Init(void);

// 底盘闭环控制更新（应在1kHz定时中断中调用）
void Chassis_Loop_Update(void);

#endif /* __CHASSIS_SOLVER_H */ 