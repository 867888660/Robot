#include "control_mode.h"
#include "control.h"        // Legacy
#include "fsm_control.h"    // JSON-FSM
#include "joystick.h"


u16 ADC_Data[2]; // ADC 数据存储

/**************************************************
函数名称：JoyStick_ADC_Init
函数功能：初始化摇杆 ADC（DMA + 连续转换）
输入参数：无
返回参数：无
***************************************************/
void JoyStick_ADC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//һGPIO_InitTypeDef͵Ľṹ
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//GPIO˿ʱ
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);//ADC1ʱ
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //ADCƵ6 72M/6=12,ADCʱ䲻ܳ14M
	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1; //PA0~PA1
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;//ģ
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;//ֻʹһADC,öģʽ
	ADC_InitStructure.ADC_ScanConvMode=ENABLE;//ɨģʽ
	ADC_InitStructure.ADC_ContinuousConvMode=ENABLE;//תģʽ
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//ⲿ
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;//תҶ
	ADC_InitStructure.ADC_NbrOfChannel=2;//תͨ2
	ADC_Init(ADC1,&ADC_InitStructure);//ʼADC
	
	ADC_Cmd(ADC1,ENABLE);
	
	ADC_DMACmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);//ʼADCУ׼Ĵ 
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
  ADC_RegularChannelConfig(ADC1,ADC_Channel_1, 1, ADC_SampleTime_239Cycles5 );		
	
	while(ADC_GetResetCalibrationStatus(ADC1));//ȴADCУ׼Ĵʼ
	
	ADC_StartCalibration(ADC1);	 //ADCУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //ȴУ׼
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//ûвⲿʹ
	
}


/**************************************************
ƣJoyStick_DMA_Init()
ܣҡͨDMA
ڲ
ز
***************************************************/
void JoyStick_DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);//DMAʱ
	
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&(ADC1->DR);//Դݵַ,
	DMA_InitStructure.DMA_MemoryBaseAddr=(uint32_t)&ADC_Data;//Ŀַ,һ
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralSRC;//:洢()洢
	DMA_InitStructure.DMA_BufferSize=2;//С
	DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;//ַַֻһ
	DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;//�������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;//�ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode=DMA_Mode_Circular;//DMAģʽ��һ��Normal��ѭ��Circular
	
	DMA_InitStructure.DMA_Priority=DMA_Priority_High;//���ȼ�:��
	DMA_InitStructure.DMA_M2M=DMA_M2M_Disable;//ʹ���ڴ浽�ڴ�Ĵ���
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); //����DMAͨ��
	DMA_Cmd(DMA1_Channel1,ENABLE);//ʹ��DMA
	
}

/**************************************************
�������ƣ�JoyStick_Key_Init()
�������ܣ�ҡ�˰�����ʼ������
��ڲ�������
���ز�������
***************************************************/
void JoyStick_Key_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;                    //����GPIO�ṹ��    
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);    //ʹ��GPIOBʱ��  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;              //��������    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������    
  GPIO_Init(GPIOB,&GPIO_InitStructure);       //��ʼ��GPIOB
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0); //����GPIOΪ�ж�����
	
	EXTI_InitStructure.EXTI_Line=EXTI_Line0;						//�ж���0
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;		//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;//�½��ش���
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;          		//ʹ���ж�
	
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	                        //��ʼ��NVIC�Ĵ���


}


/**************************************************
�������ƣ�JoyStick_Init()
�������ܣ�ҡ�˳�ʼ������
��ڲ�������
���ز�������
***************************************************/
void JoyStick_Init(void)
{
	JoyStick_Key_Init();
	JoyStick_ADC_Init();
	JoyStick_DMA_Init();
}


/**************************************************
�������ƣ�EXTI0_IRQHandler(void)
�������ܣ��ⲿ�ж�0����
��ڲ�������
���ز�������
***************************************************/
void EXTI0_IRQHandler(void)//�жϷ�����
{
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
	{
		LED=~LED;
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

/**************************************************
�������ƣ�JoyStick_Key_Scan(int mode)
�������ܣ�ҡ�˰���ɨ�躯��
��ڲ�����mode 0 ��֧��������  1 ֧��������
���ز�����1 ��������
          0 ����û�а���
***************************************************/
int JoyStick_Key_Scan(int mode)
{
    static u8 key_up=1;
    if(mode) key_up=1;
    if(key_up && (KEY_Z==1))
    {
       delay_ms(10);
       key_up=0;
       if(KEY_Z==1) 
           return 1;
    }
    else if(KEY_Z==0) 
        key_up=1;
    return 0;
}


/**************************************************
�������ƣ�JoyStick_Text()
�������ܣ�ҡ�˲��Ժ���
��ڲ�������
���ز�������
***************************************************/
void JoyStick_Text()
{
		u32 adcx,adcy;
		u8 key;
		key=JoyStick_Key_Scan(0);
	  if(key)
		{
			printf("JoyStick_Key YES!");
			printf("\n");
		}
		adcx= ADC_Data[0];			
		adcy= ADC_Data[1];	
		printf("adcx=%.3f,  adcy=%.3f\r\n", (adcx/4095.0)*3.3 , (adcy/4095.0)*3.3);  
		printf("\r\n");
		delay_ms(500);
}


static uint32_t start_press_tick = 0;
static uint8_t  in_switch_mode   = 0;
static uint8_t  last_start_key   = 0;

void JoyStick_Process(void)
{
    uint8_t start_key = KEY_START;   // 高电平 = 松开（取决于上拉/下拉）
    uint16_t rx = ADC_Data[0];       // 右摇杆 X
    uint16_t ry = ADC_Data[1];       // 右摇杆 Y

    // ---------- 1. 检测长按 ----------
    if(!start_key && last_start_key) {                // 刚按下
        start_press_tick = millis();
    }
    if(!start_key && (millis() - start_press_tick > 1000)) {
        in_switch_mode = 1;        // 超过 1 s 进入切换模式
    }

    // ---------- 2. 切换逻辑 ----------
    static uint8_t last_dir = 0;
    if(in_switch_mode) {
        int dir = 0;   // +1 = 上，-1 = 下
        if(ry > 3500) dir = +1;      // 根据 ADC 范围阈值调整
        if(ry <  500) dir = -1;

        if(dir != 0 && dir != last_dir) {   // 边沿检测，防抖
            if(dir > 0) CtrlMode_Next();    // JSON → Joystick → …
            else        CtrlMode_Prev();
        }
        last_dir = dir;
    }

    // ---------- 3. 退出 ----------
    if(start_key && !last_start_key) {      // 松开
        in_switch_mode = 0;                 // 退出并保持当前模式
    }

    last_start_key = start_key;
}

volatile CtrlMode g_ctrlMode = CTRL_MODE_JOYSTICK;

void CtrlMode_Set(CtrlMode m)
{
    if(g_ctrlMode == m) return;
    g_ctrlMode = m;
    printf("CTRL MODE -> %s\r\n",
          m == CTRL_MODE_JOYSTICK ? "JOYSTICK" : "JSON");
}

void CtrlMode_Next(void) { CtrlMode_Set((g_ctrlMode+1) % 2); }
void CtrlMode_Prev(void) { CtrlMode_Set((g_ctrlMode+1) % 2); } // 只有两档






