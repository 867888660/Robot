#include "chassis_solver.h"
#include "fsm_hardware.h"
#include "math.h"

/*
 * 底盘参数 (示例值, 请根据实际机械结构调整)
 * R : 轮半径 (m)
 * L : 底盘中心到轮的几何距离 (m)
 * K_PWM : 速度 (m/s) 到 PWM 的线性映射系数 (0-500)
 */
#define R_WHEEL    0.03f        // 6 cm 轮直径
#define L_CHASSIS  0.12f        // 24 cm 对角线一半
#define K_PWM      500.0f       // 1 m/s → 500 PWM

// PID控制参数
#define PID_KP     8.0f         // 比例系数
#define PID_KI     2.0f         // 积分系数
#define PID_KD     0.5f         // 微分系数
#define PID_IMAX   200.0f       // 积分限幅
#define PID_OUTPUT_MAX 500.0f   // 输出限幅

// 速度目标值和当前值
static float g_target_vx = 0.0f;
static float g_target_vy = 0.0f;
static float g_target_w  = 0.0f;

// PID控制结构体
typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    float target;       // 目标值
    float current;      // 当前值
    float error;        // 当前误差
    float error_last;   // 上一次误差
    float integral;     // 积分项
    float derivative;   // 微分项
    float output;       // 输出值
    float imax;         // 积分限幅
    float output_max;   // 输出限幅
} PID_Control_t;

// 电机PID控制器
static PID_Control_t motor_pid[4];

// 轮子目标速度 (m/s)
static float wheel_target_speed[4] = {0.0f};

// PID控制器初始化
static void PID_Init(PID_Control_t *pid, float kp, float ki, float kd, float imax, float output_max)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->target = 0.0f;
    pid->current = 0.0f;
    pid->error = 0.0f;
    pid->error_last = 0.0f;
    pid->integral = 0.0f;
    pid->derivative = 0.0f;
    pid->output = 0.0f;
    pid->imax = imax;
    pid->output_max = output_max;
}

// PID控制计算
static float PID_Calculate(PID_Control_t *pid, float target, float current)
{
    // 更新目标值和当前值
    pid->target = target;
    pid->current = current;
    
    // 计算误差
    pid->error = pid->target - pid->current;
    
    // 计算积分项
    pid->integral += pid->error;
    
    // 积分限幅
    if(pid->integral > pid->imax) pid->integral = pid->imax;
    if(pid->integral < -pid->imax) pid->integral = -pid->imax;
    
    // 计算微分项
    pid->derivative = pid->error - pid->error_last;
    
    // 计算PID输出
    pid->output = pid->Kp * pid->error + pid->Ki * pid->integral + pid->Kd * pid->derivative;
    
    // 输出限幅
    if(pid->output > pid->output_max) pid->output = pid->output_max;
    if(pid->output < -pid->output_max) pid->output = -pid->output_max;
    
    // 保存当前误差
    pid->error_last = pid->error;
    
    return pid->output;
}

// ================= 公共接口 =================
void Chassis_SetVelocity(float vx, float vy, float w)
{
    g_target_vx = vx;
    g_target_vy = vy;
    g_target_w  = w;
}

// 初始化底盘
void Chassis_Init(void)
{
    // 初始化PID控制器
    for(int i = 0; i < 4; i++) {
        PID_Init(&motor_pid[i], PID_KP, PID_KI, PID_KD, PID_IMAX, PID_OUTPUT_MAX);
    }
    
    // 初始化编码器
    FSM_HW_InitEncoders();
    
    // 设置初始速度为0
    g_target_vx = 0.0f;
    g_target_vy = 0.0f;
    g_target_w = 0.0f;
}

// 闭环控制实现
void Chassis_Loop_Update(void)
{
    /*
     * 典型麦克纳姆轮速度求解:
     *  V1 = vx - vy - (L + W) * w
     *  V2 = vx + vy + (L + W) * w
     *  V3 = vx + vy - (L + W) * w
     *  V4 = vx - vy + (L + W) * w
     *  由于 L = W 时可合并为 L2 = 2*L
     *  这里使用简化版 L2 = L_CHASSIS
     */
    const float L2 = L_CHASSIS;

    // 计算各轮目标线速度 (m/s)
    wheel_target_speed[0] = g_target_vx - g_target_vy - L2 * g_target_w;
    wheel_target_speed[1] = g_target_vx + g_target_vy + L2 * g_target_w;
    wheel_target_speed[2] = g_target_vx + g_target_vy - L2 * g_target_w;
    wheel_target_speed[3] = g_target_vx - g_target_vy + L2 * g_target_w;

    // 获取当前实际轮速
    float wheel_current_speed[4];
    for(int i = 0; i < 4; i++) {
        wheel_current_speed[i] = FSM_HW_GetWheelSpeed(i);
    }
    
    // 执行PID控制
    int16_t pwm_outputs[4];
    for(int i = 0; i < 4; i++) {
        // PID计算输出PWM值
        pwm_outputs[i] = (int16_t)PID_Calculate(&motor_pid[i], wheel_target_speed[i], wheel_current_speed[i]);
    }
    
    // 输出到电机
    FSM_HW_SetMotorRaw(pwm_outputs[0], pwm_outputs[1], pwm_outputs[2], pwm_outputs[3]);
} 