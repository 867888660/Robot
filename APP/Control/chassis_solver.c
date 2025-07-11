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

static float g_target_vx = 0.0f;
static float g_target_vy = 0.0f;
static float g_target_w  = 0.0f;

// ================= 公共接口 =================
void Chassis_SetVelocity(float vx, float vy, float w)
{
    g_target_vx = vx;
    g_target_vy = vy;
    g_target_w  = w;
}

// 简化版本: 直接按线性比例映射到 PWM, 无闭环
static void SolveAndDrive(void)
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

    float v1 = g_target_vx - g_target_vy - L2 * g_target_w;
    float v2 = g_target_vx + g_target_vy + L2 * g_target_w;
    float v3 = g_target_vx + g_target_vy - L2 * g_target_w;
    float v4 = g_target_vx - g_target_vy + L2 * g_target_w;

    // 映射到 PWM (-500 .. 500)
    int16_t pwm1 = (int16_t)(v1 * K_PWM);
    int16_t pwm2 = (int16_t)(v2 * K_PWM);
    int16_t pwm3 = (int16_t)(v3 * K_PWM);
    int16_t pwm4 = (int16_t)(v4 * K_PWM);

    // 饱和
    if(pwm1 > 500) pwm1 = 500; if(pwm1 < -500) pwm1 = -500;
    if(pwm2 > 500) pwm2 = 500; if(pwm2 < -500) pwm2 = -500;
    if(pwm3 > 500) pwm3 = 500; if(pwm3 < -500) pwm3 = -500;
    if(pwm4 > 500) pwm4 = 500; if(pwm4 < -500) pwm4 = -500;

    FSM_HW_SetMotorRaw(pwm1, pwm2, pwm3, pwm4);
}

void Chassis_Loop_Update(void)
{
    // 此处可加入编码器反馈 + PID 调速 – 这里先直接运行开环驱动
    SolveAndDrive();
} 