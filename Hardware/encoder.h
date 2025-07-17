#ifndef __ENCODER_H
#define __ENCODER_H

#include "sys.h"

// 编码器配置选项
// 取消注释以下行可以启用TIM4/TIM5作为编码器输入
// #define USE_TIM4_ENCODER    // 使用TIM4作为电机3编码器
// #define USE_TIM5_ENCODER    // 使用TIM5作为电机4编码器

// 电机编码器通道定义
#define ENCODER_MOTOR_1    0
#define ENCODER_MOTOR_2    1
#define ENCODER_MOTOR_3    2
#define ENCODER_MOTOR_4    3
#define ENCODER_MAX_NUM    4

// 根据实际编码器和电机参数修改这两个值
#define ENCODER_PULSE_PER_REV    11   // 每转脉冲数 (示例值，根据实际编码器调整)
#define ENCODER_GEAR_RATIO       34   // 减速比 (示例值，根据实际电机调整)

// 编码器初始化函数
void Encoder_Init(void);

// 编码器计数器清零
void Encoder_Reset(u8 motor_id);

// 读取编码器计数
s16 Encoder_GetCount(u8 motor_id);

// 计算电机RPM (每分钟转速)
float Encoder_ReadRPM(u8 motor_id);

// 更新所有编码器数据 (在定时器中断中调用)
void Encoder_UpdateAll(void);

#endif // __ENCODER_H 