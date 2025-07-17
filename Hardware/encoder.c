#include "encoder.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include "usart.h"

// 编码器计数缓存
static s16 encoder_count[ENCODER_MAX_NUM] = {0};

// 编码器上次计数值 (用于计算速度)
static s16 encoder_last_count[ENCODER_MAX_NUM] = {0};

// 电机转速 (RPM)
static float motor_rpm[ENCODER_MAX_NUM] = {0.0f};

// 编码器更新时间间隔 (ms)
static u32 encoder_update_interval = 50;

// 上次更新时间戳
static u32 last_update_time = 0;

/**
 * @brief 编码器计数器初始化 (使用定时器2和定时器3)
 * @note  TIM2用于编码器1和2，TIM3用于编码器3和4
 */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置TIM2 CH1 (PA0) 和 CH2 (PA1) - 电机1编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置TIM3 CH1 (PA6) 和 CH2 (PA7) - 电机2编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置TIM2 CH3 (PA2) 和 CH4 (PA3) - 电机3编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置TIM3 CH3 (PB0) 和 CH4 (PB1) - 电机4编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置TIM2 - 用于电机1和电机3编码器
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 配置为编码器模式
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    // 配置TIM2输入捕获
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    
    // 配置TIM3 - 用于电机2和电机4编码器
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    
    // 清零计数器
    TIM_SetCounter(TIM2, 0);
    TIM_SetCounter(TIM3, 0);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    
    printf("Encoder initialized\r\n");
}

/**
 * @brief 编码器计数器清零
 * @param motor_id 电机ID (0-3)
 */
void Encoder_Reset(u8 motor_id)
{
    if(motor_id >= ENCODER_MAX_NUM) return;
    
    // 清零计数器
    encoder_count[motor_id] = 0;
    encoder_last_count[motor_id] = 0;
    motor_rpm[motor_id] = 0;
}

/**
 * @brief 读取编码器计数
 * @param motor_id 电机ID (0-3)
 * @return 编码器计数值
 */
s16 Encoder_GetCount(u8 motor_id)
{
    if(motor_id >= ENCODER_MAX_NUM) return 0;
    
    return encoder_count[motor_id];
}

/**
 * @brief 计算电机RPM
 * @param motor_id 电机ID (0-3)
 * @return 电机转速 (RPM)
 */
float Encoder_ReadRPM(u8 motor_id)
{
    if(motor_id >= ENCODER_MAX_NUM) return 0;
    
    return motor_rpm[motor_id];
}

/**
 * @brief 更新所有编码器数据 (在定时器中断中调用)
 */
void Encoder_UpdateAll(void)
{
    extern volatile u32 g_systick_ms; // 使用全局系统滴答计数器
    u32 current_time = g_systick_ms;
    float time_diff_sec;
    s16 delta;
    
    // 检查是否需要更新
    if(current_time - last_update_time < encoder_update_interval) {
        return;
    }
    
    // 计算时间差 (秒)
    time_diff_sec = (current_time - last_update_time) / 1000.0f;
    last_update_time = current_time;
    
    // 读取TIM2和TIM3计数器
    s16 tim2_cnt = (s16)TIM_GetCounter(TIM2);
    s16 tim3_cnt = (s16)TIM_GetCounter(TIM3);
    
    // 更新电机1编码器计数 (TIM2 CH1/CH2)
    encoder_count[ENCODER_MOTOR_1] = tim2_cnt;
    
    // 更新电机2编码器计数 (TIM3 CH1/CH2)
    encoder_count[ENCODER_MOTOR_2] = tim3_cnt;
    
    // 暂时使用模拟数据更新电机3和电机4的编码器计数
    // TODO: 实现全部4个编码器的读取
    encoder_count[ENCODER_MOTOR_3] = encoder_count[ENCODER_MOTOR_1];
    encoder_count[ENCODER_MOTOR_4] = encoder_count[ENCODER_MOTOR_2];
    
    // 计算每个电机的RPM
    for(u8 i = 0; i < ENCODER_MAX_NUM; i++) {
        // 计算脉冲变化量
        delta = encoder_count[i] - encoder_last_count[i];
        
        // 计算RPM: (delta / PULSE_PER_REV) / GEAR_RATIO * (60 / time_diff_sec)
        motor_rpm[i] = (float)delta / ENCODER_PULSE_PER_REV / ENCODER_GEAR_RATIO * (60.0f / time_diff_sec);
        
        // 更新上次计数值
        encoder_last_count[i] = encoder_count[i];
    }
} 