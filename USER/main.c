#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "fsm_control.h"

/****************************************************************************
智能小车主控引脚说明

N20电机：
STBY1:PB4    STBY2:PB9
1:PA2,PB6      2:PA3,PB5
3:PA0,PB8      4:PA1,PB7

蓝牙模块：PA9,PA10

NRF24L01 无线模块
CE->PB13  CSN->PB14  IRQ->PB15  SCK->PA5  MISO->PA6  MOSI->PA7  

|-----------------|  IRQ   MISO 
|       NRF     ::|  MOSI  SCK                       
|      24L01    ::|  CSN   CE
|-----------------|  VCC   GND

超声波模块
TRIG:PB10         ECHO:PB11

RGB灯带
DI:PB0

************************************************************************************/

// 系统滴答计数器，用于时间戳
volatile u32 g_systick_ms = 0;

// 传感器数据更新标志
static u8 g_update_sensors_flag = 0;

// SysTick中断处理函数
void SysTick_Handler(void)
{
    g_systick_ms++;
    
    // 每50ms更新一次传感器数据
    if(g_systick_ms % 50 == 0) {
        g_update_sensors_flag = 1;
    }
}

// 初始化系统
void SystemInit(void)
{
    // 初始化延时函数
    delay_init();
    
    // 初始化中断分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LED
    LED_Init();
    
    // 初始化串口
    USART1_Init(115200);
    
    // 初始化电机
    Motor_Init();
    
    // 初始化超声波
    Hcsr04_Init();
    
    // 初始化NRF24L01
    NRF24L01_Init();
    NRF24L01_Check_detection(); // NRF24L01检测响应
    
    // 初始化RGB LED
    RGB_LED_Init();
    
    // 配置SysTick中断，1ms中断一次
    SysTick_Config(SystemCoreClock / 1000);
    
    // 关闭电机输出
    Motion_State(OFF);
    
    printf("System initialized\r\n");
}

int main(void)
{   
    // 初始化系统
    SystemInit();
    
    // 初始化OLED
    OLED_Init();
    OLED_Clear();
    
    // 显示欢迎信息
    OLED_ShowString(0, 0, (u8*)"FSM Robot v1.0", 16);
    OLED_ShowString(0, 2, (u8*)"Initializing...", 16);
    delay_ms(1000);
    OLED_Clear();
    
    // 初始化状态机
    FSM_Init();
    
    // 加载默认规则
    FSM_LoadDefaultRules();
    
    printf("Robot started in state %d\r\n", FSM_GetCurrentState());
    
    // 主循环
    while(1)
    {
        // 更新传感器数据
        if(g_update_sensors_flag) {
            FSM_UpdateSensors();
            g_update_sensors_flag = 0;
        }
        
        // 更新状态机
        FSM_Update();
        
        // 发送心跳（状态机内部会控制发送频率）
        FSM_SendHeartbeat();
        
        // 延时一小段时间，避免过于频繁的更新
        delay_ms(10);
    }
} 