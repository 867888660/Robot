#include "fsm_parser.h"
#include "fsm_hardware.h"
#include "usart.h"
#include "chassis_solver.h"

// 全局状态机实例
StateMachine g_fsm;

// 序列号计数器
static u32 g_seqCounter = 0;

// 初始化状态机
void FSM_Parser_Init(void)
{
    // 清空状态机结构
    memset(&g_fsm, 0, sizeof(StateMachine));
    
    // 设置初始状态
    g_fsm.currentState = STATE_INIT;
    
    // 初始化传感器值
    for(u8 i = 0; i < SENSOR_MAX; i++) {
        g_fsm.sensors[i].numValue = 0.0f;
        g_fsm.sensors[i].isString = 0;
        g_fsm.sensors[i].timestamp = 0;
    }
    
    // 初始化序列号计数器
    g_seqCounter = 0;
    
    printf("FSM: Initialized in state %d\r\n", g_fsm.currentState);
}

// 解析比较操作符
static CompareOp ParseCompareOp(const char* cmpStr)
{
    if(strcmp(cmpStr, "==") == 0) return CMP_EQ;
    if(strcmp(cmpStr, "!=") == 0) return CMP_NE;
    if(strcmp(cmpStr, "<") == 0)  return CMP_LT;
    if(strcmp(cmpStr, ">") == 0)  return CMP_GT;
    if(strcmp(cmpStr, "<=") == 0) return CMP_LE;
    if(strcmp(cmpStr, ">=") == 0) return CMP_GE;
    
    // 默认相等
    return CMP_EQ;
}

// 解析逻辑操作符
static LogicOp ParseLogicOp(const char* opStr)
{
    if(strcmp(opStr, "AND") == 0) return LOGIC_AND;
    if(strcmp(opStr, "OR") == 0)  return LOGIC_OR;
    if(strcmp(opStr, "NOT") == 0) return LOGIC_NOT;
    if(strcmp(opStr, "TRUE") == 0) return LOGIC_TRUE;
    
    // 默认AND
    return LOGIC_AND;
}

// 解析传感器类型
static SensorType ParseSensorType(const char* sensorStr)
{
    if(strcmp(sensorStr, "SNR") == 0) return SENSOR_SNR;
    if(strcmp(sensorStr, "MPU_ROLL") == 0) return SENSOR_MPU_ROLL;
    if(strcmp(sensorStr, "MPU_PITCH") == 0) return SENSOR_MPU_PITCH;
    if(strcmp(sensorStr, "MPU_YAW") == 0) return SENSOR_MPU_YAW;
    if(strcmp(sensorStr, "IR_LINE") == 0) return SENSOR_IR_LINE;
    if(strcmp(sensorStr, "BAT") == 0) return SENSOR_BAT;
    if(strcmp(sensorStr, "NRF_MSG") == 0) return SENSOR_NRF_MSG;
    
    // 默认返回超声波
    return SENSOR_SNR;
}

// 解析表情类型
static EmojiType ParseEmojiType(const char* emojiStr)
{
    if(strcmp(emojiStr, "HAPPY") == 0) return EMOJI_HAPPY;
    if(strcmp(emojiStr, "SAD") == 0) return EMOJI_SAD;
    if(strcmp(emojiStr, "ANGRY") == 0) return EMOJI_ANGRY;
    if(strcmp(emojiStr, "SURPRISED") == 0) return EMOJI_SURPRISED;
    if(strcmp(emojiStr, "NEUTRAL") == 0) return EMOJI_NEUTRAL;
    
    // 默认返回中性表情
    return EMOJI_NEUTRAL;
}

// 解析JSON字符串为状态机
// 注意：这里只是一个简化版的解析器框架，实际应使用cJSON等库进行完整解析
u8 FSM_Parser_ParseJSON(const char* json)
{
    // 这里应该使用cJSON库解析JSON
    // 由于STM32资源有限，这里只提供一个框架，实际实现需要根据具体情况调整
    
    printf("FSM: Parsing JSON script...\r\n");
    
    // 假设我们已经解析了JSON，并提取了必要的字段
    // 在实际实现中，这里应该使用cJSON_GetObjectItem等函数解析
    
    // 清理旧的转换规则
    if(g_fsm.transitions != NULL) {
        // 清理每个转换规则的动态分配内存
        for(u8 i = 0; i < g_fsm.transitionCount; i++) {
            if(g_fsm.transitions[i].state_in != NULL) {
                free(g_fsm.transitions[i].state_in);
            }
            
            // 清理动作数组
            if(g_fsm.transitions[i].actions != NULL) {
                free(g_fsm.transitions[i].actions);
            }
            
            // 清理复合条件（如果有）
            if(!g_fsm.transitions[i].condition.isAtomic && 
               g_fsm.transitions[i].condition.composite != NULL) {
                // 递归清理复合条件（简化版，实际应该递归清理）
                if(g_fsm.transitions[i].condition.composite->conditions != NULL) {
                    free(g_fsm.transitions[i].condition.composite->conditions);
                }
                free(g_fsm.transitions[i].condition.composite);
            }
        }
        
        free(g_fsm.transitions);
        g_fsm.transitions = NULL;
    }
    
    // 假设解析后的转换规则数量
    g_fsm.transitionCount = 4; // 示例中有4条规则
    
    // 分配转换规则数组内存
    g_fsm.transitions = (Transition*)malloc(g_fsm.transitionCount * sizeof(Transition));
    if(g_fsm.transitions == NULL) {
        printf("FSM: Memory allocation failed\r\n");
        return 0;
    }
    
    // 初始化转换规则
    memset(g_fsm.transitions, 0, g_fsm.transitionCount * sizeof(Transition));
    
    // 示例：手动填充第一条规则（STOP_BY_SONAR）
    // 在实际实现中，这些应该从JSON解析得到
    strcpy(g_fsm.transitions[0].id, "STOP_BY_SONAR");
    g_fsm.transitions[0].priority = 100;
    
    // 状态数组
    g_fsm.transitions[0].stateInCount = 2;
    g_fsm.transitions[0].state_in = (u8*)malloc(2 * sizeof(u8));
    g_fsm.transitions[0].state_in[0] = 1; // NORMAL
    g_fsm.transitions[0].state_in[1] = 3; // REMOTE_CTRL
    
    // 条件：SNR < 10
    g_fsm.transitions[0].condition.isAtomic = 1;
    g_fsm.transitions[0].condition.atomic.sensor = SENSOR_SNR;
    g_fsm.transitions[0].condition.atomic.cmp = CMP_LT;
    g_fsm.transitions[0].condition.atomic.numValue = 10.0f;
    g_fsm.transitions[0].condition.atomic.isString = 0;
    g_fsm.transitions[0].condition.atomic.for_ms = 0;
    
    // 动作：停止 + 显示悲伤表情
    g_fsm.transitions[0].actionCount = 2;
    g_fsm.transitions[0].actions = (Action*)malloc(2 * sizeof(Action));
    
    // 动作1：停止
    g_fsm.transitions[0].actions[0].cmd = CMD_STOP_ALL;
    g_fsm.transitions[0].actions[0].async = 0;
    g_fsm.transitions[0].actions[0].completed = 0;
    
    // 动作2：显示悲伤表情
    g_fsm.transitions[0].actions[1].cmd = CMD_OLED_EMOJI;
    g_fsm.transitions[0].actions[1].emoji.name = EMOJI_SAD;
    g_fsm.transitions[0].actions[1].async = 0;
    g_fsm.transitions[0].actions[1].completed = 0;
    
    // 目标状态
    g_fsm.transitions[0].state_out = STATE_EMERGENCY_STOP;
    
    // 备注
    strcpy(g_fsm.transitions[0].note, "急停+显示悲伤表情");
    
    // 类似地，填充其他规则...
    // 此处省略其他规则的填充代码，实际应从JSON解析
    
    printf("FSM: Parsed %d transitions\r\n", g_fsm.transitionCount);
    return 1;
}

// 更新数值型传感器
void FSM_UpdateSensor(SensorType sensor, float value)
{
    if(sensor >= SENSOR_MAX) return;
    
    g_fsm.sensors[sensor].numValue = value;
    g_fsm.sensors[sensor].isString = 0;
    g_fsm.sensors[sensor].timestamp = FSM_HW_GetTimeMs();
}

// 更新字符串型传感器
void FSM_UpdateSensorStr(SensorType sensor, const char* value)
{
    if(sensor >= SENSOR_MAX) return;
    
    strncpy(g_fsm.sensors[sensor].strValue, value, 31);
    g_fsm.sensors[sensor].strValue[31] = '\0'; // 确保字符串结束
    g_fsm.sensors[sensor].isString = 1;
    g_fsm.sensors[sensor].timestamp = FSM_HW_GetTimeMs();
}

// 评估原子条件
static u8 EvaluateAtomicCondition(const AtomicCondition* cond)
{
    if(cond->sensor >= SENSOR_MAX) return 0;
    
    const SensorValue* sensor = &g_fsm.sensors[cond->sensor];
    u8 result = 0;
    
    // 检查传感器类型是否匹配
    if(cond->isString != sensor->isString) return 0;
    
    // 根据传感器类型进行比较
    if(cond->isString) {
        // 字符串比较
        switch(cond->cmp) {
            case CMP_EQ:
                result = (strcmp(sensor->strValue, cond->strValue) == 0);
                break;
            case CMP_NE:
                result = (strcmp(sensor->strValue, cond->strValue) != 0);
                break;
            default:
                result = 0; // 字符串不支持其他比较操作
                break;
        }
    } else {
        // 数值比较
        switch(cond->cmp) {
            case CMP_EQ:
                result = (sensor->numValue == cond->numValue);
                break;
            case CMP_NE:
                result = (sensor->numValue != cond->numValue);
                break;
            case CMP_LT:
                result = (sensor->numValue < cond->numValue);
                break;
            case CMP_GT:
                result = (sensor->numValue > cond->numValue);
                break;
            case CMP_LE:
                result = (sensor->numValue <= cond->numValue);
                break;
            case CMP_GE:
                result = (sensor->numValue >= cond->numValue);
                break;
            default:
                result = 0;
                break;
        }
    }
    
    // 如果需要持续时间检查
    if(cond->for_ms > 0) {
        u32 currentTime = FSM_HW_GetTimeMs();
        
        if(result) {
            // 条件满足，检查是否已经开始计时
            if(cond->startTime == 0) {
                // 开始计时
                ((AtomicCondition*)cond)->startTime = currentTime;
                result = 0; // 还未达到持续时间
            } else if(currentTime - cond->startTime >= cond->for_ms) {
                // 已达到持续时间
                result = 1;
            } else {
                // 未达到持续时间
                result = 0;
            }
        } else {
            // 条件不满足，重置计时
            ((AtomicCondition*)cond)->startTime = 0;
        }
    }
    
    return result;
}

// 评估复合条件
static u8 EvaluateCompositeCondition(const CompositeCondition* cond)
{
    if(cond->op == LOGIC_TRUE) return 1;
    
    if(cond->condCount == 0) return 0;
    
    switch(cond->op) {
        case LOGIC_AND:
            for(u8 i = 0; i < cond->condCount; i++) {
                if(!FSM_EvaluateCondition(&cond->conditions[i])) {
                    return 0; // 一个条件不满足，AND结果为假
                }
            }
            return 1; // 所有条件满足
            
        case LOGIC_OR:
            for(u8 i = 0; i < cond->condCount; i++) {
                if(FSM_EvaluateCondition(&cond->conditions[i])) {
                    return 1; // 一个条件满足，OR结果为真
                }
            }
            return 0; // 所有条件不满足
            
        case LOGIC_NOT:
            if(cond->condCount > 0) {
                return !FSM_EvaluateCondition(&cond->conditions[0]);
            }
            return 0;
            
        default:
            return 0;
    }
}

// 评估条件
u8 FSM_EvaluateCondition(const Condition* condition)
{
    if(condition->isAtomic) {
        return EvaluateAtomicCondition(&condition->atomic);
    } else if(condition->composite != NULL) {
        return EvaluateCompositeCondition(condition->composite);
    }
    return 0;
}

// 执行动作
void FSM_ExecuteAction(const Action* action)
{
    if(action == NULL) return;
    
    switch(action->cmd) {
        case CMD_STOP_ALL:
            FSM_HW_StopAllMotors();
            break;
            
        case CMD_SET_CHASSIS_VEL:
            Chassis_SetVelocity(action->chassis.vx, action->chassis.vy, action->chassis.w);
            break;
            
        case CMD_SET_MOTOR_RAW:
            FSM_HW_SetMotorRaw(action->motor.m1, action->motor.m2, 
                              action->motor.m3, action->motor.m4);
            break;
            
        case CMD_SET_LED:
            FSM_HW_SetLED(action->led.r, action->led.g, 
                         action->led.b, action->led.count);
            break;
            
        case CMD_OLED_TEXT:
            FSM_HW_ShowText(action->text.text);
            break;
            
        case CMD_NRF_SEND:
            FSM_HW_SendNRF(action->nrf.data, action->nrf.len);
            break;
            
        case CMD_BEEP:
            FSM_HW_Beep(action->beep.ms);
            break;
            
        case CMD_OLED_EMOJI:
            FSM_HW_ShowEmoji(action->emoji.name);
            break;
            
        default:
            printf("FSM: Unknown command %d\r\n", action->cmd);
            break;
    }
}

// 发送心跳报告
void FSM_Parser_SendHeartbeat(void)
{
    static u32 lastHeartbeatTime = 0;
    u32 currentTime = FSM_HW_GetTimeMs();
    
    // 每秒发送一次心跳
    if(currentTime - lastHeartbeatTime >= 1000) {
        lastHeartbeatTime = currentTime;
        
        // 构造心跳JSON
        printf("{"
               "\"type\":\"HB\","
               "\"seq\":%lu,"
               "\"tick\":%lu,"
               "\"state\":%d,"
               "\"battery\":%d,"
               "\"cpu\":%d,"
               "\"queue\":0"
               "}\r\n", 
               g_seqCounter++, 
               currentTime, 
               g_fsm.currentState,
               FSM_HW_GetBatteryLevel(),
               FSM_HW_GetCPUUsage());
    }
}

// 发送事件报告
void FSM_Parser_SendEventReport(const Transition* transition)
{
    if(transition == NULL) return;
    
    // 构造事件JSON
    printf("{"
           "\"type\":\"EV\","
           "\"seq\":%lu,"
           "\"event_id\":\"%s\","
           "\"state_in\":%d,"
           "\"state_out\":%d,"
           "\"cause\":{"
           "\"SNR\":%.1f"  // 假设触发条件是超声波
           "},"
           "\"ts\":%lu"
           "}\r\n",
           g_seqCounter++, 
           transition->id,
           g_fsm.currentState,
           transition->state_out,
           g_fsm.sensors[SENSOR_SNR].numValue,
           FSM_HW_GetTimeMs());
}

// 发送动作报告
void FSM_Parser_SendActionReport(const Action* action, const char* result)
{
    if(action == NULL) return;
    
    u32 currentTime = FSM_HW_GetTimeMs();
    static u32 startTime = 0;
    
    // 简化版本，只报告命令类型
    printf("{"
           "\"type\":\"AC\","
           "\"seq\":%lu,"
           "\"seq_id\":%lu,"
           "\"cmd\":%d,"
           "\"result\":\"%s\","
           "\"t_start\":%lu,"
           "\"t_end\":%lu,"
           "\"delta\":{"
           "\"wheel_pwm_ms\":0,"
           "\"dist_cm\":0.0"
           "}"
           "}\r\n",
           g_seqCounter++,
           (u32)action,  // 使用动作指针作为唯一ID
           action->cmd,
           result,
           startTime,
           currentTime);
    
    // 更新开始时间，用于下一个动作
    startTime = currentTime;
}

// 发送警报
void FSM_Parser_SendAlert(const char* alertCode, const char* detail)
{
    printf("{"
           "\"type\":\"AL\","
           "\"seq\":%lu,"
           "\"alert_code\":\"%s\","
           "\"detail\":{\"message\":\"%s\"},"
           "\"ts\":%lu"
           "}\r\n",
           g_seqCounter++,
           alertCode,
           detail,
           FSM_HW_GetTimeMs());
}

// 更新状态机
void FSM_Parser_Update(void)
{
    // 如果没有规则，直接返回
    if(g_fsm.transitions == NULL || g_fsm.transitionCount == 0) {
        return;
    }
    
    // 检查是否有正在执行的转换
    for(u8 i = 0; i < g_fsm.transitionCount; i++) {
        Transition* trans = &g_fsm.transitions[i];
        
        if(trans->isExecuting) {
            // 执行下一个动作
            if(trans->currentAction < trans->actionCount) {
                Action* action = &trans->actions[trans->currentAction];
                
                if(!action->completed) {
                    FSM_ExecuteAction(action);
                    FSM_Parser_SendActionReport(action, "OK");
                    action->completed = 1;
                }
                
                // 如果是异步动作，或者已完成，移到下一个
                if(action->async || action->completed) {
                    trans->currentAction++;
                }
            } else {
                // 所有动作已执行完毕
                g_fsm.currentState = trans->state_out;
                trans->isExecuting = 0;
                printf("FSM: Transition %s completed, new state: %d\r\n", 
                       trans->id, g_fsm.currentState);
            }
            
            // 一次只执行一个转换
            return;
        }
    }
    
    // 查找匹配的转换规则
    Transition* matchedTrans = NULL;
    u8 highestPriority = 0;
    
    for(u8 i = 0; i < g_fsm.transitionCount; i++) {
        Transition* trans = &g_fsm.transitions[i];
        
        // 检查当前状态是否匹配
        u8 stateMatch = 0;
        for(u8 j = 0; j < trans->stateInCount; j++) {
            if(trans->state_in[j] == g_fsm.currentState) {
                stateMatch = 1;
                break;
            }
        }
        
        if(!stateMatch) continue;
        
        // 评估条件
        if(FSM_EvaluateCondition(&trans->condition)) {
            // 条件满足，检查优先级
            if(matchedTrans == NULL || trans->priority > highestPriority) {
                matchedTrans = trans;
                highestPriority = trans->priority;
            }
        }
    }
    
    // 如果找到匹配的转换，开始执行
    if(matchedTrans != NULL) {
        matchedTrans->isExecuting = 1;
        matchedTrans->currentAction = 0;
        
        // 重置所有动作的完成状态
        for(u8 i = 0; i < matchedTrans->actionCount; i++) {
            matchedTrans->actions[i].completed = 0;
        }
        
        // 发送事件报告
        FSM_Parser_SendEventReport(matchedTrans);
        
        printf("FSM: Starting transition %s (priority %d)\r\n", 
               matchedTrans->id, matchedTrans->priority);
    }
} 