#include "fsm_hardware.h"
#include "motor.h"
#include "hcsr.h"
#include "ws2812b.h"
#include "oled.h"
#include "nrf24l01.h"
#include "mpu6050.h"
#include "delay.h"

// 系统滴答计数器，在main.c中定义和更新
extern volatile u32 g_systick_ms;

// 最近接收到的NRF消息
static char g_nrf_message[32] = {0};

// 电机控制 - 停止所有电机
void FSM_HW_StopAllMotors(void)
{
    Motion_State(OFF); // 关闭所有电机
}

// 电机控制 - 移动指定距离
void FSM_HW_MoveDistance(int16_t dist_cm, u8 speed)
{
    if(dist_cm > 0) {
        forward(speed);
        delay_ms(dist_cm * 100); // 简化：假设速度为100时，1cm需要100ms
        Motion_State(OFF);
    } else if(dist_cm < 0) {
        backward(speed);
        delay_ms(-dist_cm * 100);
        Motion_State(OFF);
    }
}

// 电机控制 - 转向指定角度
void FSM_HW_Turn(int16_t angle_deg, u8 speed)
{
    if(angle_deg > 0) {
        Right_Turn(speed);
        delay_ms(angle_deg * 10); // 简化：假设速度为100时，1度需要10ms
        Motion_State(OFF);
    } else if(angle_deg < 0) {
        Left_Turn(speed);
        delay_ms(-angle_deg * 10);
        Motion_State(OFF);
    }
}

// 电机控制 - 直接设置电机PWM值
void FSM_HW_SetMotorRaw(int16_t m1, int16_t m2, int16_t m3, int16_t m4)
{
    // 左前轮
    if(m1 >= 0) {
        TIM_SetCompare3(TIM2, m1);
        L_AIN2_OFF;
    } else {
        TIM_SetCompare3(TIM2, -m1);
        L_AIN2_ON;
    }
    
    // 左后轮
    if(m2 >= 0) {
        TIM_SetCompare4(TIM2, m2);
        L_BIN2_OFF;
    } else {
        TIM_SetCompare4(TIM2, -m2);
        L_BIN2_ON;
    }
    
    // 右前轮
    if(m3 >= 0) {
        TIM_SetCompare1(TIM2, m3);
        R_AIN2_OFF;
    } else {
        TIM_SetCompare1(TIM2, -m3);
        R_AIN2_ON;
    }
    
    // 右后轮
    if(m4 >= 0) {
        TIM_SetCompare2(TIM2, m4);
        R_BIN2_OFF;
    } else {
        TIM_SetCompare2(TIM2, -m4);
        R_BIN2_ON;
    }
}

// 三自由度底盘速度控制 (vx, vy, ω)
void FSM_HW_SetChassisVelocity(float vx, float vy, float w)
{
    /*
     * 简化实现：仅根据 vx 方向控制前进/后退速度；
     * vy、w 暂未使用。真实项目应根据麦克纳姆/全向轮运动学计算各轮 PWM。
     */
    int16_t pwm = (int16_t)(vx * 500); // 假设 vx ∈ [-1,1] m/s 对应 PWM [-500,500]
    if(pwm > 500) pwm = 500;
    if(pwm < -500) pwm = -500;

    if(pwm > 0) {
        forward(pwm);
    } else if(pwm < 0) {
        backward(-pwm);
    } else {
        Motion_State(OFF);
    }
    // TODO: vy 与 w 控制横移与旋转，可在此补充全向轮算法
}

// LED控制 - 设置RGB LED
void FSM_HW_SetLED(u8 r, u8 g, u8 b, u8 count)
{
    for(u8 i = 0; i < count; i++) {
        RGB_LED_Write_24Bits(r, g, b);
    }
}

// OLED控制 - 显示文本
void FSM_HW_ShowText(const char* text)
{
    OLED_ShowString(0, 0, (u8*)text, 16);
}

// OLED控制 - 显示表情
void FSM_HW_ShowEmoji(EmojiType emoji)
{
    // 这里应该根据表情类型显示不同的表情图案
    // 简化版本，只显示表情名称
    switch(emoji) {
        case EMOJI_HAPPY:
            OLED_ShowString(0, 2, (u8*)"HAPPY", 16);
            break;
        case EMOJI_SAD:
            OLED_ShowString(0, 2, (u8*)"SAD", 16);
            break;
        case EMOJI_ANGRY:
            OLED_ShowString(0, 2, (u8*)"ANGRY", 16);
            break;
        case EMOJI_SURPRISED:
            OLED_ShowString(0, 2, (u8*)"SURPRISED", 16);
            break;
        case EMOJI_NEUTRAL:
            OLED_ShowString(0, 2, (u8*)"NEUTRAL", 16);
            break;
        default:
            break;
    }
}

// 无线通信 - 发送NRF数据
void FSM_HW_SendNRF(const u8* data, u8 len)
{
    NRF24L01_TX_Mode();
    NRF24L01_TxPacket((u8*)data);
}

// 蜂鸣器控制
void FSM_HW_Beep(u16 ms)
{
    // 假设有一个蜂鸣器控制函数
    // Beep_On();
    delay_ms(ms);
    // Beep_Off();
}

// 传感器读取 - 获取超声波距离
float FSM_HW_GetSonarDistance(void)
{
    return Hcsr04GetLength();
}

// 传感器读取 - 获取MPU6050数据
void FSM_HW_GetMPUData(float* pitch, float* roll, float* yaw)
{
    // 这里应该调用MPU6050的读取函数
    // 简化版本，使用模拟数据
    *pitch = 0.0f;
    *roll = 0.0f;
    *yaw = 0.0f;
}

// 传感器读取 - 获取红外线状态
const char* FSM_HW_GetIRLineStatus(void)
{
    // 这里应该调用TCRT5000的读取函数
    // 简化版本，使用模拟数据
    return "WHITE";
}

// 传感器读取 - 获取电池电量
u8 FSM_HW_GetBatteryLevel(void)
{
    // 这里应该读取电池电量
    // 简化版本，使用模拟数据
    return 86; // 86%
}

// 传感器读取 - 获取NRF消息
const char* FSM_HW_GetNRFMessage(void)
{
    u8 rx_buf[33] = {0};
    
    // 尝试接收NRF消息
    NRF24L01_RX_Mode();
    if(NRF24L01_RxPacket(rx_buf) == 0) {
        rx_buf[32] = '\0'; // 确保字符串结束
        strcpy(g_nrf_message, (char*)rx_buf);
    }
    
    return g_nrf_message;
}

// 系统时间 - 获取毫秒计数
u32 FSM_HW_GetTimeMs(void)
{
    return g_systick_ms;
}

// 系统状态 - 获取CPU使用率
u8 FSM_HW_GetCPUUsage(void)
{
    // 这里应该计算CPU使用率
    // 简化版本，使用模拟数据
    return 24; // 24%
}

// 系统状态 - 获取队列长度
u8 FSM_HW_GetQueueLength(void)
{
    // 这里应该返回动作队列长度
    // 简化版本，使用模拟数据
    return 0;
} 