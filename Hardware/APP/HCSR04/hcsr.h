#ifndef __HCSR_H
#define __HCSR_H	 
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stdlib.h"


//硬件接口定义
#define HCSR04_PORT     GPIOB
#define HCSR04_CLK      RCC_APB2Periph_GPIOB
#define HCSR04_TRIG     GPIO_Pin_10
#define HCSR04_ECHO     GPIO_Pin_11

#define TRIG_Send  PBout(10) 
#define ECHO_Reci  PBin(11)

// 定义超时时间，基于距离计算
// 最大探测距离为4米时，声波往返时间约为24ms (4*2/340*1000)
#define HCSR04_MAX_DISTANCE     400  // 最大距离400cm
#define HCSR04_TIMEOUT_MS       30   // 超时时间30ms
#define HCSR04_FILTER_SIZE      5    // 滤波采样数量
#define HCSR04_DELAY_BETWEEN_SAMPLES 10 // 采样间隔10ms

void Hcsr04_NVIC(void);
void Hcsr04_Init(void);
// 移除static函数声明，这些函数应该只在.c文件中定义
u32 GetEchoTimer(void);//获取定时器4计数值
float Hcsr04GetLength(void);//通过定时器4计数值计算距离

// 新增函数
float Hcsr04_GetDistance(void);  // 与main.c保持一致的函数命名
float Hcsr04_FilteredMeasurement(void); // 添加滤波测量
u8 Hcsr04_MeasureWithTimeout(float* distance); // 带超时的测量函数

void Hcsr04_Text(void);
		 				    
#endif



