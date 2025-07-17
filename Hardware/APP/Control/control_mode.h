#ifndef __CONTROL_MODE_H
#define __CONTROL_MODE_H

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

// 控制模式枚举
typedef enum {
    CTRL_MODE_MANUAL = 0,   // 手动控制模式
    CTRL_MODE_AUTO,         // 自动控制模式
    CTRL_MODE_JOYSTICK      // 摇杆控制模式
} CtrlMode;

// 全局控制模式变量
extern volatile CtrlMode g_ctrlMode;

// 函数声明
void CtrlMode_Set(CtrlMode m);
void CtrlMode_Next(void);
void CtrlMode_Prev(void);

#endif /* __CONTROL_MODE_H */ 