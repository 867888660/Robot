#include "hcsr.h"


u16 msHcCount = 0;


/**************************************************
函数名称：Hcsr04_NVIC(void)
函数功能：NVIC设置
输入参数：无
返回参数：无
***************************************************/
void Hcsr04_NVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
			
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;             
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       
	NVIC_Init(&NVIC_InitStructure);
}


/**************************************************
函数名称：Hcsr04_Init(void)
函数功能：IO口初始化 定时器初始化
输入参数：无
返回参数：无
***************************************************/
void Hcsr04_Init(void)
{  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;   
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(HCSR04_CLK, ENABLE);
   
    GPIO_InitStructure.GPIO_Pin =HCSR04_TRIG;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HCSR04_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(HCSR04_PORT,HCSR04_TRIG);
     
    GPIO_InitStructure.GPIO_Pin =   HCSR04_ECHO;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(HCSR04_PORT, &GPIO_InitStructure);  
    GPIO_ResetBits(HCSR04_PORT,HCSR04_ECHO);    
     
          
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   
     
    TIM_DeInit(TIM4);
    TIM_TimeBaseStructure.TIM_Period = (1000-1); 
    TIM_TimeBaseStructure.TIM_Prescaler =(72-1); 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);          


    TIM_ClearFlag(TIM4, TIM_FLAG_Update);  
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);    
    Hcsr04_NVIC();
    TIM_Cmd(TIM4,DISABLE);     
}

/**************************************************
函数名称：OpenTimerForHc()  
函数功能：打开定时器4
输入参数：无
返回参数：无
***************************************************/
static void OpenTimerForHc()  
{
   TIM_SetCounter(TIM4,0);
   msHcCount = 0;
   TIM_Cmd(TIM4, ENABLE); 
}


/**************************************************
函数名称：void CloseTimerForHc()  
函数功能：关闭定时器4
输入参数：无
返回参数：无
***************************************************/
static void CloseTimerForHc()    
{
   TIM_Cmd(TIM4, DISABLE); 
}


/**************************************************
函数名称：TIM4_IRQHandler(void)   
函数功能：定时器4中断
输入参数：无
返回参数：无
***************************************************/
void TIM4_IRQHandler(void)  
{
   if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  
   {
       TIM_ClearITPendingBit(TIM4, TIM_IT_Update  ); 
       msHcCount++;
   }
}
 
/**************************************************
函数名称：GetEchoTimer(void)  
函数功能：获取定时器4计数值
输入参数：无
返回参数：t:计数值
***************************************************/
u32 GetEchoTimer(void)
{
   u32 t = 0;
   t = msHcCount*1000;
   t += TIM_GetCounter(TIM4);
   TIM4->CNT = 0;  
   delay_ms(50);
   return t;
}


/**************************************************
函数名称：Hcsr04GetLength(void)
函数功能：通过定时器4计数值计算距离
输入参数：无
返回参数：lengthTemp:距离CM
***************************************************/
float Hcsr04GetLength(void)
{
   u32 t = 0;
   int i = 0;
   float lengthTemp = 0;
   float sum = 0;
   while(i!=5)
   {
      TRIG_Send = 1;      
      delay_us(20);
      TRIG_Send = 0;
      while(ECHO_Reci == 0);   
      OpenTimerForHc();        
      i = i + 1;
      while(ECHO_Reci == 1);
      CloseTimerForHc();        
      t = GetEchoTimer();        
      lengthTemp = ((float)t/58.0);//cm
      sum = lengthTemp + sum;
      delay_ms(10); // 增加测量之间的延时，提高精度
   }
   lengthTemp = sum/5.0;
   return lengthTemp;
}

/**************************************************
函数名称：Hcsr04_MeasureWithTimeout(float* distance)
函数功能：带超时机制的超声波测距
输入参数：distance - 测量距离的指针
返回参数：0-测量超时，1-测量成功
***************************************************/
u8 Hcsr04_MeasureWithTimeout(float* distance)
{
   u32 t = 0;
   u32 timeout = 0;
   
   // 发送10us以上的高电平脉冲触发测距
   TRIG_Send = 1;      
   delay_us(20);
   TRIG_Send = 0;
   
   // 等待Echo引脚变高，增加超时处理
   timeout = 0;
   while(ECHO_Reci == 0)
   {
      delay_us(10);
      timeout++;
      if(timeout > 1000) // 10ms超时，Echo未变高
      {
         *distance = HCSR04_MAX_DISTANCE; // 返回最大距离
         return 0; // 超时返回0
      }
   }
   
   // 开始计时
   OpenTimerForHc();
   
   // 等待Echo引脚变低，增加超时处理
   timeout = 0;
   while(ECHO_Reci == 1)
   {
      delay_us(100);
      timeout++;
      if(timeout > HCSR04_TIMEOUT_MS * 10) // 超过设定的超时时间
      {
         CloseTimerForHc();
         *distance = HCSR04_MAX_DISTANCE; // 返回最大距离
         return 0; // 超时返回0
      }
   }
   
   // 停止计时
   CloseTimerForHc();
   
   // 获取计时结果
   t = GetEchoTimer();
   
   // 计算距离 (声速340m/s, t/58 = 厘米)
   *distance = ((float)t/58.0);
   
   // 范围限制
   if(*distance > HCSR04_MAX_DISTANCE)
      *distance = HCSR04_MAX_DISTANCE;
   
   return 1; // 测量成功
}

/**************************************************
函数名称：Hcsr04_FilteredMeasurement(void)
函数功能：带滤波的超声波测距
输入参数：无
返回参数：滤波后的距离值(cm)
***************************************************/
float Hcsr04_FilteredMeasurement(void)
{
   float distances[HCSR04_FILTER_SIZE];
   float validDistances[HCSR04_FILTER_SIZE];
   float sum = 0;
   u8 validCount = 0;
   u8 i, j, k;
   float temp;
   
   // 连续测量多次
   for(i = 0; i < HCSR04_FILTER_SIZE; i++)
   {
      if(Hcsr04_MeasureWithTimeout(&distances[i]))
      {
         validDistances[validCount++] = distances[i];
      }
      delay_ms(HCSR04_DELAY_BETWEEN_SAMPLES); // 测量间隔
   }
   
   // 如果没有有效测量，返回最大距离
   if(validCount == 0)
      return HCSR04_MAX_DISTANCE;
   
   // 冒泡排序去除异常值
   for(i = 0; i < validCount - 1; i++)
   {
      for(j = 0; j < validCount - 1 - i; j++)
      {
         if(validDistances[j] > validDistances[j+1])
         {
            temp = validDistances[j];
            validDistances[j] = validDistances[j+1];
            validDistances[j+1] = temp;
         }
      }
   }
   
   // 去掉最高和最低值后计算均值（如果有足够多的有效测量）
   k = 0;
   if(validCount >= 3)
   {
      // 去掉最高和最低
      for(i = 1; i < validCount - 1; i++)
      {
         sum += validDistances[i];
         k++;
      }
   }
   else
   {
      // 有效数据太少，全部使用
      for(i = 0; i < validCount; i++)
      {
         sum += validDistances[i];
         k++;
      }
   }
   
   return sum / k;
}

/**************************************************
函数名称：Hcsr04_GetDistance(void)
函数功能：对外提供的获取距离接口，与main.c中一致
输入参数：无
返回参数：距离值(cm)
***************************************************/
float Hcsr04_GetDistance(void)
{
   return Hcsr04_FilteredMeasurement();
}

/**************************************************
函数名称：Hcsr04_Text(void)
函数功能：测试函数
输入参数：无
返回参数：无
***************************************************/
void Hcsr04_Text(void)
{
	float r;
	r = Hcsr04_GetDistance();
	printf("r=%.2f cm\n", r);
	delay_ms(1000);
}





