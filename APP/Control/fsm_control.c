#include "fsm_control.h"
#include "fsm_hardware.h"
#include "FSM_Parser.h"
#include "usart.h"

// 外部声明状态机实例
extern StateMachine g_fsm;

// 示例JSON规则
const char* default_json = 
"{\n"
"  \"script_ver\":\"2025-07-05-alpha3\",\n"
"  \"transitions\":[\n"
"    { \"id\":\"STOP_BY_SONAR\",\"priority\":100,\"state_in\":[1,3],\n"
"      \"when\":{\"sensor\":\"SNR\",\"cmp\":\"<\",\"value\":10},\n"
"      \"actions\":[{\"cmd\":0},{\"cmd\":8,\"name\":\"SAD\"}],\n"
"      \"state_out\":2,\"note\":\"急停+显示悲伤表情\" },\n"
"\n"
"    { \"id\":\"LINE_FOLLOW\",\"priority\":80,\"state_in\":[1],\n"
"      \"when\":{\"sensor\":\"IR_LINE\",\"cmp\":\"==\",\"value\":\"BLACK\"},\n"
"      \"actions\":[{\"cmd\":1,\"dist\":2,\"spd\":120}],\n"
"      \"state_out\":1 },\n"
"\n"
"    { \"id\":\"LED_IDLE\",\"priority\":10,\"state_in\":[1],\n"
"      \"when\":{\"op\":\"TRUE\"},\n"
"      \"actions\":[{\"cmd\":4,\"r\":0,\"g\":0,\"b\":255,\"async\":true},{\"cmd\":8,\"name\":\"HAPPY\",\"async\":true}],\n"
"      \"state_out\":1 },\n"
"\n"
"    { \"id\":\"DEFAULT_SAFE\",\"priority\":0,\"state_in\":[1,2,3],\n"
"      \"when\":{\"op\":\"TRUE\"},\n"
"      \"actions\":[{\"cmd\":0}],\n"
"      \"state_out\":2 }\n"
"  ]\n"
"}\n";

// 初始化状态机
void FSM_Init(void)
{
    // 调用底层初始化函数
    FSM_Parser_Init();
    
    printf("FSM: Control interface initialized\r\n");
}

// 加载默认规则
void FSM_LoadDefaultRules(void)
{
    FSM_ParseJSON(default_json);
    
    // 设置初始状态为正常状态
    FSM_SetCurrentState(STATE_NORMAL);
    
    printf("FSM: Default rules loaded\r\n");
}

// 解析JSON规则
u8 FSM_ParseJSON(const char* json)
{
    return FSM_Parser_ParseJSON(json);
}

// 更新传感器数据
void FSM_UpdateSensors(void)
{
    // 读取超声波传感器数据
    float distance = FSM_HW_GetSonarDistance();
    FSM_UpdateSensor(SENSOR_SNR, distance);
    
    // 读取MPU6050数据
    float pitch, roll, yaw;
    FSM_HW_GetMPUData(&pitch, &roll, &yaw);
    
    FSM_UpdateSensor(SENSOR_MPU_PITCH, pitch);
    FSM_UpdateSensor(SENSOR_MPU_ROLL, roll);
    FSM_UpdateSensor(SENSOR_MPU_YAW, yaw);
    
    // 读取红外线传感器数据
    const char* ir_status = FSM_HW_GetIRLineStatus();
    FSM_UpdateSensorStr(SENSOR_IR_LINE, ir_status);
    
    // 读取电池电量
    u8 battery = FSM_HW_GetBatteryLevel();
    FSM_UpdateSensor(SENSOR_BAT, (float)battery);
    
    // 读取NRF消息
    const char* nrf_msg = FSM_HW_GetNRFMessage();
    if(nrf_msg[0] != '\0') {
        FSM_UpdateSensorStr(SENSOR_NRF_MSG, nrf_msg);
    }
}

// 更新状态机
void FSM_Update(void)
{
    // 调用底层更新函数
    FSM_Parser_Update();
}

// 获取当前状态
u8 FSM_GetCurrentState(void)
{
    return g_fsm.currentState;
}

// 设置当前状态
void FSM_SetCurrentState(u8 state)
{
    g_fsm.currentState = state;
    printf("FSM: State changed to %d\r\n", state);
}

// 发送心跳报告
void FSM_SendHeartbeat(void)
{
    FSM_Parser_SendHeartbeat();
}

// 发送警报
void FSM_SendAlert(const char* alertCode, const char* detail)
{
    FSM_Parser_SendAlert(alertCode, detail);
} 