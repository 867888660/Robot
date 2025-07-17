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
 * @brief 编码器计数器初始化 (使用定时器2、3、4和5)
 * @note  TIM2用于电机1编码器，TIM3用于电机2编码器，TIM4用于电机3编码器，TIM5用于电机4编码器
 *        如果TIM4或TIM5不可用，则使用GPIO模拟编码器
 */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
    #ifdef USE_TIM4_ENCODER
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    #endif
    #ifdef USE_TIM5_ENCODER
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    #endif
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置TIM2 CH1 (PA0) 和 CH2 (PA1) - 电机1编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置TIM3 CH1 (PA6) 和 CH2 (PA7) - 电机2编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置电机3编码器引脚 - 使用TIM4或GPIO模拟
    #ifdef USE_TIM4_ENCODER
    // 配置TIM4 CH1 (PB6) 和 CH2 (PB7) - 电机3编码器A相和B相
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    #else
    // 使用PA2和PA3作为普通GPIO进行编码器信号读取
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    #endif
    
    // 配置电机4编码器引脚 - 使用TIM5或GPIO模拟
    #ifdef USE_TIM5_ENCODER
    // 配置TIM5 CH1 (PA0) 和 CH2 (PA1) - 电机4编码器A相和B相 (注：可能与TIM2冲突，需修改)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    #else
    // 使用PB0和PB1作为普通GPIO进行编码器信号读取
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    #endif
    
    // 配置基本定时器参数
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    // 配置TIM2 - 用于电机1编码器
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    
    // 配置TIM3 - 用于电机2编码器
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    
    #ifdef USE_TIM4_ENCODER
    // 配置TIM4 - 用于电机3编码器
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    #endif
    
    #ifdef USE_TIM5_ENCODER
    // 配置TIM5 - 用于电机4编码器
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM5, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICInit(TIM5, &TIM_ICInitStructure);
    #endif
    
    // 清零计数器
    TIM_SetCounter(TIM2, 32768);  // 设置为中间值便于检测方向
    TIM_SetCounter(TIM3, 32768);
    #ifdef USE_TIM4_ENCODER
    TIM_SetCounter(TIM4, 32768);
    #endif
    #ifdef USE_TIM5_ENCODER
    TIM_SetCounter(TIM5, 32768);
    #endif
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    #ifdef USE_TIM4_ENCODER
    TIM_Cmd(TIM4, ENABLE);
    #endif
    #ifdef USE_TIM5_ENCODER
    TIM_Cmd(TIM5, ENABLE);
    #endif
    
    // 重置编码器计数数组
    for(int i = 0; i < ENCODER_MAX_NUM; i++) {
        encoder_count[i] = 0;
        encoder_last_count[i] = 0;
        motor_rpm[i] = 0.0f;
    }
    
    printf("Encoder initialized with 4-motor support\r\n");
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
    
    // 读取TIM2计数器 - 处理电机1和电机3
    TIM_SetCounter(TIM2, 32768); // 设置初始值为中间值，便于检测方向
    delay_ms(5);  // 短暂延时让编码器累计脉冲
    s16 tim2_cnt = (s16)TIM_GetCounter(TIM2);
    s16 encoder1_delta = tim2_cnt - 32768;
    
    // 读取TIM3计数器 - 处理电机2和电机4
    TIM_SetCounter(TIM3, 32768); // 设置初始值为中间值
    delay_ms(5);
    s16 tim3_cnt = (s16)TIM_GetCounter(TIM3);
    s16 encoder2_delta = tim3_cnt - 32768;
    
    // 读取TIM4计数器 - 处理电机3 (假设使用TIM4的CH1/CH2)
    // 注：为了实现全部4个编码器，这里使用TIM4作为额外的计数器
    s16 encoder3_delta = 0;
    #ifdef USE_TIM4_ENCODER
    TIM_SetCounter(TIM4, 32768);
    delay_ms(5);
    s16 tim4_cnt = (s16)TIM_GetCounter(TIM4);
    encoder3_delta = tim4_cnt - 32768;
    #else
    // 暂时使用PA2/PA3作为编码器3的输入
    // 通过GPIO读取引脚状态计算方向和脉冲
    static u8 last_a3 = 0, last_b3 = 0;
    u8 a3 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2);
    u8 b3 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
    
    if(a3 != last_a3) {
        if(b3 != a3) encoder3_delta++;
        else encoder3_delta--;
    }
    last_a3 = a3;
    last_b3 = b3;
    #endif
    
    // 读取TIM5计数器 - 处理电机4 (假设使用TIM5的CH1/CH2)
    s16 encoder4_delta = 0;
    #ifdef USE_TIM5_ENCODER
    TIM_SetCounter(TIM5, 32768);
    delay_ms(5);
    s16 tim5_cnt = (s16)TIM_GetCounter(TIM5);
    encoder4_delta = tim5_cnt - 32768;
    #else
    // 暂时使用PB0/PB1作为编码器4的输入
    static u8 last_a4 = 0, last_b4 = 0;
    u8 a4 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
    u8 b4 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
    
    if(a4 != last_a4) {
        if(b4 != a4) encoder4_delta++;
        else encoder4_delta--;
    }
    last_a4 = a4;
    last_b4 = b4;
    #endif
    
    // 更新编码器计数
    encoder_count[ENCODER_MOTOR_1] += encoder1_delta;
    encoder_count[ENCODER_MOTOR_2] += encoder2_delta;
    encoder_count[ENCODER_MOTOR_3] += encoder3_delta;
    encoder_count[ENCODER_MOTOR_4] += encoder4_delta;
    
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