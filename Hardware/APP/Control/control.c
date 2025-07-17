#include "control.h"

u8 Joy_RxBuf[20];//摇杆接收数据缓存
u8 MPU_RxBuf[10];//MPU接收数据缓存
u8 Joy_Lpos,Joy_Rpos;//摇杆左右位置
u8 MPU_Data[10];//MPU数据滤波后值
u8 Con_RxBuf[10];//控制指令接收缓存
u8 Eva_RxBuf[4];//避障停止指令缓存
u8 RGB_RxBuf[4];//RGB停止指令缓存
u8 mode=1;//模式，1为默认蓝牙模式
extern u8 mode_flag;
u8 NRF_flag=0;//无线模式退出循环标志位
u8 MPU_flag=0;//重力模式退出循环标志位

u8 RGB_flag=0;//RGB灯带标志位
u8 RGB_mode=0;//RGB灯带模式
int HSV_H=0;//HSV色调H值
u8 HSV_flag=0;//颜色转换时的标志位
u8 LED_Count=0;//LED灯数量

extern char Lx_Buf[10];//左摇杆数据缓存
extern char Rx_Buf[10];//右摇杆数据缓存

/**************************************************
函数名称：Map(int val,int in_min,int in_max,int out_min,int out_max)
函数功能：数值映射
输入参数：val:需要映射的值
          in_min:输入值的范围最小值
          in_max:输入值的范围最大值
          out_min:输出值的范围最小值
          out_max:输出值的范围最大值
返回参数：映射后的值
***************************************************/
int Map(int val,int in_min,int in_max,int out_min,int out_max)
{
	return (int)(val-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}

/**************************************************
函数名称：Bluetooth_Mode(void)
函数功能：蓝牙模式
输入参数：无
返回参数：无
***************************************************/
void Bluetooth_Mode(void)
{
	if(mode_flag==1)
	{
		APP_Joy_Mode();//APP摇杆模式
	}
	else if(mode_flag==2)APP_Gravity_Mode();//APP重力感应模式
	else if(mode_flag==3)Evadible_Mode();
	else if(mode_flag==4)Follow_Mode();
	else if(mode_flag==5)RGB_mode=0;//流水灯
	else if(mode_flag==6)RGB_mode=1;//彩虹灯
	else if(mode_flag==7)RGB_mode=2;//闪烁灯
	else Motion_State(ON);
}

/**************************************************
函数名称：Wireless_Mode(void)
函数功能：无线模式
输入参数：无
返回参数：无
***************************************************/
void Wireless_Mode(void)
{
	int Joy_Lx=50, Joy_Ly = 50, Joy_Rx = 50, Joy_Ry = 50;
	int Map_Lx, Map_Ly, Map_Rx, Map_Ry;
	int pwm1, pwm2, pwm3, pwm4;
	static u8 RGB_mode1=0,i=0;
	static u8 RGB_mode2=0,j=0;
	NRF24L01_RX_Mode();
	L_STBY_ON;
	R_STBY_ON;
	if(NRF24L01_RxPacket(Joy_RxBuf)==0)
	{
		  Joy_Lx = (Joy_RxBuf[2] - '0') * 10 + (Joy_RxBuf[3] - '0');
			Joy_Ly = (Joy_RxBuf[5] - '0') * 10 + (Joy_RxBuf[6] - '0');
			
			Joy_Rx = (Joy_RxBuf[9] - '0') * 10 + (Joy_RxBuf[10] - '0');
			Joy_Ry = (Joy_RxBuf[12] - '0') * 10 + (Joy_RxBuf[13] - '0');

			Map_Lx = Map(Joy_Lx, 10, 90, -127, 127);
			Map_Ly = Map(Joy_Ly, 10, 90, -127, 127);
			Map_Rx = Map(Joy_Rx, 10, 90, -127, 127);
			Map_Ry = Map(Joy_Ry, 10, 90, -127, 127);

			
			pwm1 = -Map_Ly - Map_Lx - Map_Ry - Map_Rx;
			pwm2 = -Map_Ly + Map_Lx - Map_Ry + Map_Rx;
			pwm3 = -Map_Ly - Map_Lx - Map_Ry + Map_Rx;
			pwm4 = -Map_Ly + Map_Lx - Map_Ry - Map_Rx;


			pwm1 = Map(pwm1, -127, 127, -499, 499);
			pwm2 = Map(pwm2, -127, 127, -499, 499);
			pwm3 = Map(pwm3, -127, 127, -499, 499);
			pwm4 = Map(pwm4, -127, 127, -499, 499);


			if (pwm1 < 20 && pwm1 >-20)pwm1 = 0;
			if (pwm2 < 20 && pwm2 >-20)pwm2 = 0;
			if (pwm3 < 20 && pwm3 >-20)pwm3 = 0;
			if (pwm4 < 20 && pwm4 >-20)pwm4 = 0;

			if (pwm1 > 499)pwm1 = 499;
			if (pwm2 > 499)pwm2 = 499;
			if (pwm3 > 499)pwm3 = 499;
			if (pwm4 > 499)pwm4 = 499;
			
			if (pwm1 < -499)pwm1 = -499;
			if (pwm2 < -499)pwm2 = -499;
			if (pwm3 < -499)pwm3 = -499;
			if (pwm4 < -499)pwm4 = -499;
			
			if(pwm1>=0)
			{
				TIM_SetCompare4(TIM2,500-pwm1);//L_BIN2:左后轮
				L_BIN2_ON;
				
			}
			else if(pwm1<0)
			{
				pwm1=abs(pwm1);
				TIM_SetCompare4(TIM2,pwm1);//L_BIN2:左后轮
				L_BIN2_OFF;	
			}
			
			if(pwm2>=0)
			{
				TIM_SetCompare3(TIM2,pwm2);//L_AIN2:左前轮
				L_AIN2_OFF;
			}
			else if(pwm2<0)
			{
				pwm2=abs(pwm2);
				TIM_SetCompare3(TIM2,500-pwm2);//L_AIN2:左前轮
				L_AIN2_ON;
			}
					
			if(pwm3>=0)
			{
				TIM_SetCompare1(TIM2,500-pwm3);//R_AIN2:右前轮
				R_AIN2_ON;
			}
			else if(pwm3<0)
			{
				pwm3=abs(pwm3);
				TIM_SetCompare1(TIM2,pwm3);//R_AIN2:右前轮
				R_AIN2_OFF;
			}
					
			if(pwm4>=0)
			{
				TIM_SetCompare2(TIM2,pwm4);//R_BIN2:右后轮
				R_BIN2_OFF;
			}
			else if(pwm4<0)
			{
				pwm4=abs(pwm4);
				TIM_SetCompare2(TIM2,500-pwm4);//R_BIN2:右后轮
				R_BIN2_ON;
			}
			
			if(Joy_RxBuf[0]=='L'&&Joy_RxBuf[3]=='R')
			{
				Motion_State(ON);
				NRF_flag=1;
			}
		printf((char*)Joy_RxBuf); 
		printf("\n"); 
	}
	else
	{	
 		printf("Receive failed !\n"); 
	}
	
	if(RGB_mode==0)
	{
		//RGB灯带流水灯效果
		hsv_to_rgb(HSV_H,100,100,&RGB_R,&RGB_G,&RGB_B);
		if(HSV_H==360)HSV_flag=1;
		else if(HSV_H==0)HSV_flag=0; 
		if(HSV_flag==0)HSV_H+=2;
		else if(HSV_flag==1)HSV_H-=2;
		for(LED_Count=0;LED_Count<6;LED_Count++)
		{
			RGB_LED_Write_24Bits(RGB_R, RGB_G, RGB_B);
		}
	}
	else if(RGB_mode==1)
	{	
		if(i%20==0)
		{
			for(LED_Count=0;LED_Count<6;LED_Count++)
			{
				hsv_to_rgb((RGB_mode1%360),100,100,&RGB_R,&RGB_G,&RGB_B);
				RGB_LED_Write_24Bits(RGB_R, RGB_G, RGB_B);
				RGB_mode1+=60;
				if(RGB_mode1>360)
				{
					RGB_mode1=0;
					i=0;
				}
			}
		}
		i++;
	}
	else if(RGB_mode==2)	
	{
		RGB_Circulation(RGB_mode2);
		if(j%20==0)RGB_mode2++;
		if(RGB_mode2>8)
		{
			j=0;
			RGB_mode2=0;
		}
		j++;
	}
	
	delay_ms(10);	
}

/**************************************************
函数名称：Evadible_Mode(void)
函数功能：避障模式
输入参数：无
返回参数：无
***************************************************/
void Evadible_Mode(void)
{
    float Dis;
    static u8 obstacle_detect_count = 0; // 连续障碍物检测计数
    static u8 direction = 0; // 0-左转，1-右转，动态变换方向
    
    Dis = Hcsr04_GetDistance(); // 使用改进后的距离测量函数
    
    // 障碍物状态判断
    if(Dis <= 15) {
        obstacle_detect_count++; // 连续检测到障碍物计数增加
        
        // 连续检测到障碍物超过阈值，执行避障
        if(obstacle_detect_count >= 2) {
            // 停止当前运动
            Motion_State(OFF);
            delay_ms(100);
            
            // 后退一小段距离
            backward(200);
            delay_ms(400);
            
            // 停止后退
            Motion_State(OFF);
            delay_ms(100);
            
            // 根据当前方向执行转向
            if(direction == 0) {
                Left_Turn(200);
                delay_ms(600);  // 转向时间增加，确保能避开障碍物
                direction = 1;  // 下次转向改为右转
            } else {
                Right_Turn(200);
                delay_ms(600);
                direction = 0;  // 下次转向改为左转
            }
            
            // 停止转向
            Motion_State(OFF);
            delay_ms(100);
            
            obstacle_detect_count = 0; // 重置计数器
        }
    } else {
        obstacle_detect_count = 0; // 未检测到障碍物，重置计数器
        forward(180); // 正常前进，速度稍微降低以提高安全性
    }
}

/**************************************************
函数名称：Gravity_Mode(void)
函数功能：重力模式
输入参数：无
返回参数：无
***************************************************/
void Gravity_Mode(void)
{
	int MPU_pitch=50, MPU_roll = 50;
	int Map_pitch, Map_roll;
	int pwm1, pwm2, pwm3, pwm4;
	static u8 RGB_mode1=0,i=0;
	static u8 RGB_mode2=0,j=0;
	NRF24L01_RX_Mode();
	L_STBY_ON;
	R_STBY_ON;
	if(NRF24L01_RxPacket(MPU_RxBuf)==0)
	{
		  MPU_pitch = (MPU_RxBuf[1] - '0') * 10 + (MPU_RxBuf[2] - '0');
			MPU_roll = (MPU_RxBuf[4] - '0') * 10 + (MPU_RxBuf[5] - '0');
						
			Map_pitch = Map(MPU_pitch, 10, 90, -127, 127);
			Map_roll = Map(MPU_roll, 10, 90, -127, 127);
				
			pwm1 =  Map_pitch + Map_roll;
			pwm2 =  Map_pitch - Map_roll;
			pwm3 =  Map_pitch + Map_roll;
			pwm4 =  Map_pitch - Map_roll;


			pwm1 = Map(pwm1, -127, 127, -499, 499);
			pwm2 = Map(pwm2, -127, 127, -499, 499);
			pwm3 = Map(pwm3, -127, 127, -499, 499);
			pwm4 = Map(pwm4, -127, 127, -499, 499);


			if (pwm1 < 20 && pwm1 >-20)pwm1 = 0;
			if (pwm2 < 20 && pwm2 >-20)pwm2 = 0;
			if (pwm3 < 20 && pwm3 >-20)pwm3 = 0;
			if (pwm4 < 20 && pwm4 >-20)pwm4 = 0;

			if (pwm1 > 499)pwm1 = 499;
			if (pwm2 > 499)pwm2 = 499;
			if (pwm3 > 499)pwm3 = 499;
			if (pwm4 > 499)pwm4 = 499;
			
			if (pwm1 < -499)pwm1 = -499;
			if (pwm2 < -499)pwm2 = -499;
			if (pwm3 < -499)pwm3 = -499;
			if (pwm4 < -499)pwm4 = -499;
			
			if(pwm1>=0)
			{
				TIM_SetCompare4(TIM2,500-pwm1);//L_BIN2:左后轮
				L_BIN2_ON;
				
			}
			else if(pwm1<0)
			{
				pwm1=abs(pwm1);
				TIM_SetCompare4(TIM2,pwm1);//L_BIN2:左后轮
				L_BIN2_OFF;	
			}
			
			if(pwm2>=0)
			{
				TIM_SetCompare3(TIM2,pwm2);//L_AIN2:左前轮
				L_AIN2_OFF;
			}
			else if(pwm2<0)
			{
				pwm2=abs(pwm2);
				TIM_SetCompare3(TIM2,500-pwm2);//L_AIN2:左前轮
				L_AIN2_ON;
			}
					
			if(pwm3>=0)
			{
				TIM_SetCompare1(TIM2,500-pwm3);//R_AIN2:右前轮
				R_AIN2_ON;
			}
			else if(pwm3<0)
			{
				pwm3=abs(pwm3);
				TIM_SetCompare1(TIM2,pwm3);//R_AIN2:右前轮
				R_AIN2_OFF;
			}
					
			if(pwm4>=0)
			{
				TIM_SetCompare2(TIM2,pwm4);//R_BIN2:右后轮
				R_BIN2_OFF;
			}
			else if(pwm4<0)
			{
				pwm4=abs(pwm4);
				TIM_SetCompare2(TIM2,500-pwm4);//R_BIN2:右后轮
				R_BIN2_ON;
			}
					
			
			if(MPU_RxBuf[0]=='M'&&MPU_RxBuf[1]=='P'&&MPU_RxBuf[2]=='U')
			{
				Motion_State(ON);
				MPU_flag=1;
			}
	}
	else
	{	
 		printf("Receive failed !\n"); 
	}
	
	if(RGB_mode==0)
	{
		//RGB灯带流水灯效果
		hsv_to_rgb(HSV_H,100,100,&RGB_R,&RGB_G,&RGB_B);
		if(HSV_H==360)HSV_flag=1;
		else if(HSV_H==0)HSV_flag=0; 
		if(HSV_flag==0)HSV_H+=2;
		else if(HSV_flag==1)HSV_H-=2;
		for(LED_Count=0;LED_Count<6;LED_Count++)
		{
			RGB_LED_Write_24Bits(RGB_R, RGB_G, RGB_B);
		}
	}
	else if(RGB_mode==1)
	{	
		if(i%20==0)
		{
			for(LED_Count=0;LED_Count<6;LED_Count++)
			{
				hsv_to_rgb((RGB_mode1%360),100,100,&RGB_R,&RGB_G,&RGB_B);
				RGB_LED_Write_24Bits(RGB_R, RGB_G, RGB_B);
				RGB_mode1+=60;
				if(RGB_mode1>360)
				{
					RGB_mode1=0;
					i=0;
				}
			}
		}
		i++;
	}
	else if(RGB_mode==2)	
	{
		RGB_Circulation(RGB_mode2);
		if(j%20==0)RGB_mode2++;
		if(RGB_mode2>8)
		{
			j=0;
			RGB_mode2=0;
		}
		j++;
	}
	
	delay_ms(10);	
}

/**************************************************
函数名称：Follow_Mode(void)
函数功能：跟随模式
输入参数：无
返回参数：无
***************************************************/
void Follow_Mode(void)
{
    float Dis;
    static float prev_distance = 0;
    static u8 stable_count = 0;
    
    Dis = Hcsr04_GetDistance(); // 使用改进后的距离测量函数
    
    // 目标距离区间
    const float TARGET_MIN = 15.0f;
    const float TARGET_MAX = 25.0f;
    
    // 防止首次读取导致剧烈变化
    if(prev_distance == 0) {
        prev_distance = Dis;
        return;
    }
    
    // 距离变化率计算（简单滤波）
    float distance_change = Dis - prev_distance;
    prev_distance = Dis;
    
    // 检测距离稳定性
    if(fabs(distance_change) < 3.0f) {
        stable_count++;
    } else {
        stable_count = 0;
    }
    
    // 根据距离调整行为
    if(Dis < TARGET_MIN) {
        // 距离太近，后退
        backward(150 + (int)(10.0f * (TARGET_MIN - Dis))); // 速度与距离成正比
    }
    else if(Dis > TARGET_MAX) {
        // 距离太远，前进
        if(Dis > 100) {
            // 超出跟随范围，停止
            Motion_State(OFF);
        } else {
            forward(150 + (int)(2.0f * (Dis - TARGET_MAX))); // 速度与距离成正比
        }
    }
    else {
        // 在目标范围内，保持静止
        Motion_State(OFF);
    }
    
    // 如果距离变化剧烈，可能是障碍物突然移动，减速以防碰撞
    if(fabs(distance_change) > 10.0f && stable_count < 3) {
        Motion_State(OFF);
        delay_ms(100);  // 短暂暂停以等待稳定
    }
}

/**************************************************
函数名称：RGB_Select(void)
函数功能：RGB灯带模式选择
输入参数：无
返回参数：无
***************************************************/
void RGB_Select(void)
{
	Motion_State(ON);
	NRF24L01_RX_Mode();
	if(NRF24L01_RxPacket(RGB_RxBuf)==0&&RGB_RxBuf[0]!=' ')
	{
		if(RGB_RxBuf[0]=='S'&&RGB_RxBuf[1]=='T'&&RGB_RxBuf[2]=='O'&&RGB_RxBuf[3]=='P')
		{
			RGB_flag=1;
		}
		if(RGB_RxBuf[2]=='0')RGB_mode=0;//流水灯
		else if(RGB_RxBuf[2]=='1')RGB_mode=1;//彩虹灯
		else if(RGB_RxBuf[2]=='2')RGB_mode=2;//闪烁灯
		
	}
	delay_ms(10);
	printf("RGB_mode=%d\n",RGB_mode);
}

/**************************************************
函数名称：RGB_Show(void)
函数功能：RGB灯带显示
输入参数：无
返回参数：无
***************************************************/
void RGB_Show(void)
{
	static u8 RGB_mode1=0,i=0;
	static u8 RGB_mode2=0,j=0;
	if(RGB_mode==0)
	{
			//RGB灯带流水灯效果
		hsv_to_rgb(HSV_H,100,100,&RGB_R,&RGB_G,&RGB_B);
		if(HSV_H==360)HSV_flag=1;
		else if(HSV_H==0)HSV_flag=0; 
		if(HSV_flag==0)HSV_H+=10;
		else if(HSV_flag==1)HSV_H-=10;
		for(LED_Count=0;LED_Count<6;LED_Count++)
		{
			RGB_LED_Write_24Bits(RGB_R, RGB_G, RGB_B);
		}
		delay_ms(100);
	}
	else if(RGB_mode==1)
	{	
		if(i%3==0)
		{
			for(LED_Count=0;LED_Count<6;LED_Count++)
			{
				hsv_to_rgb((RGB_mode1%360),100,100,&RGB_R,&RGB_G,&RGB_B);
				RGB_LED_Write_24Bits(RGB_R, RGB_G, RGB_B);
				RGB_mode1+=60;
				if(RGB_mode1>360)
				{
					RGB_mode1=0;
					i=0;
				}
			}
		}
		i++;
		delay_ms(100);
	}
	else if(RGB_mode==2)	
	{
		RGB_Circulation(RGB_mode2);
		if(j%3==0)RGB_mode2++;
		if(RGB_mode2>8)
		{
			j=0;
			RGB_mode2=0;
		}
		j++;
		delay_ms(100);
	}
}

/**************************************************
函数名称：APP_Joy_Mode(void)
函数功能：APP摇杆模式
输入参数：无
返回参数：无
***************************************************/
void APP_Joy_Mode(void)
{
	int Joy_Lx=50, Joy_Ly = 50, Joy_Rx = 50, Joy_Ry = 50;
	int Map_Lx, Map_Ly, Map_Rx, Map_Ry;
	int pwm1, pwm2, pwm3, pwm4;
	
	if (Lx_Buf[0] == 'L')
	{
		Joy_Lx = (Lx_Buf[2] - '0') * 10 + (Lx_Buf[3] - '0');
		Joy_Ly = (Lx_Buf[5] - '0') * 10 + (Lx_Buf[6] - '0');
	}
	if (Rx_Buf[0] == 'R')
	{
		Joy_Rx = (Rx_Buf[2] - '0') * 10 + (Rx_Buf[3] - '0');
		Joy_Ry = (Rx_Buf[5] - '0') * 10 + (Rx_Buf[6] - '0');
	}
	
	Map_Lx = Map(Joy_Lx, 10, 90, -127, 127);
	Map_Ly = Map(Joy_Ly, 10, 90, -127, 127);
	Map_Rx = Map(Joy_Rx, 10, 90, -127, 127);
	Map_Ry = Map(Joy_Ry, 10, 90, -127, 127);

	
	pwm1 = -Map_Ly + Map_Lx - Map_Ry + Map_Rx;
	pwm2 = -Map_Ly - Map_Lx - Map_Ry - Map_Rx;
	pwm3 = -Map_Ly + Map_Lx - Map_Ry - Map_Rx;
	pwm4 = -Map_Ly - Map_Lx - Map_Ry + Map_Rx;


	pwm1 = Map(pwm1, -127, 127, -499, 499);
	pwm2 = Map(pwm2, -127, 127, -499, 499);
	pwm3 = Map(pwm3, -127, 127, -499, 499);
	pwm4 = Map(pwm4, -127, 127, -499, 499);


	if (pwm1 < 20 && pwm1 >-20)pwm1 = 0;
	if (pwm2 < 20 && pwm2 >-20)pwm2 = 0;
	if (pwm3 < 20 && pwm3 >-20)pwm3 = 0;
	if (pwm4 < 20 && pwm4 >-20)pwm4 = 0;

	if (pwm1 > 499)pwm1 = 499;
	if (pwm2 > 499)pwm2 = 499;
	if (pwm3 > 499)pwm3 = 499;
	if (pwm4 > 499)pwm4 = 499;
	
	if (pwm1 < -499)pwm1 = -499;
	if (pwm2 < -499)pwm2 = -499;
	if (pwm3 < -499)pwm3 = -499;
	if (pwm4 < -499)pwm4 = -499;
	
	
	if(pwm1>=0)
	{
		TIM_SetCompare4(TIM2,500-pwm1);//L_BIN2:左后轮
		L_BIN2_ON;
				
	}
	else if(pwm1<0)
	{
		pwm1=abs(pwm1);
		TIM_SetCompare4(TIM2,pwm1);//L_BIN2:左后轮
		L_BIN2_OFF;	
	}
	
	if(pwm2>=0)
	{
		TIM_SetCompare3(TIM2,pwm2);//L_AIN2:左前轮
		L_AIN2_OFF;
	}
	else if(pwm2<0)
	{
		pwm2=abs(pwm2);
		TIM_SetCompare3(TIM2,500-pwm2);//L_AIN2:左前轮
		L_AIN2_ON;
	}
			
	if(pwm3>=0)
	{
		TIM_SetCompare1(TIM2,500-pwm3);//R_AIN2:右前轮
		R_AIN2_ON;
	}
	else if(pwm3<0)
	{
		pwm3=abs(pwm3);
		TIM_SetCompare1(TIM2,pwm3);//R_AIN2:右前轮
		R_AIN2_OFF;
	}
			
	if(pwm4>=0)
	{
		TIM_SetCompare2(TIM2,pwm4);//R_BIN2:右后轮
		R_BIN2_OFF;
	}
	else if(pwm4<0)
	{
		pwm4=abs(pwm4);
		TIM_SetCompare2(TIM2,500-pwm4);//R_BIN2:右后轮
		R_BIN2_ON;
	}
	delay_ms(10);
//	printf(Lx_Buf);
//	printf(Rx_Buf);
//	printf(Rx_Buf);
//	printf("\n");
}

/**************************************************
函数名称：APP_Gravity_Mode(void)
函数功能：APP重力感应模式
输入参数：无
返回参数：无
***************************************************/
void APP_Gravity_Mode(void)
{
	int i,j=0,Pitch_flag=0;
	int APP_Pitch=0,APP_Roll=0;
	int Pitch_symbel=1,Roll_symbel=1;//偏移符号
	char Pitch_Buf[10],Roll_Buf[10];
	int Map_pitch, Map_roll;//映射偏移量
	int pwm1, pwm2, pwm3, pwm4;
	static int Smoothing_Pitch_Buf[5];//平滑滤波缓存
	static int Smoothing_Roll_Buf[5];//平滑滤波缓存
	static int Smoothing_Count=0;//平滑滤波计数
	int Pitch_temp,Roll_temp;//选择值
	
	L_STBY_ON;
	R_STBY_ON;
	
	//获取Roll
	for(i=1;i<20;i++)
	{
		if(Pitch_Roll_Buf[i]=='.')break;
		Roll_Buf[i-1]=Pitch_Roll_Buf[i];
	}
	//获取Pitch
	for(i=0;i<20;i++)
	{
		if(Pitch_Roll_Buf[i]==',')
		{
			Pitch_flag=1;
			i++;
		}
		if(Pitch_flag==1)
		{
			if(Pitch_Roll_Buf[i]=='.')
			{
				j=0;
				break;
			}
			Pitch_Buf[j]=Pitch_Roll_Buf[i];
			j++;
		}
	}
	//将Roll字符串转换为数字
	j=0;
	for(i=10;i>=0;i--)
	{
		if(Roll_Buf[i]>='0'&&Roll_Buf[i]<='9')
		{
			APP_Roll+=(Roll_Buf[i]-'0')*pow(10,j);
			j++;
		}
		if(Roll_Buf[0]=='-')
		{
			Roll_symbel=-1;
		}
	}
	//将Pitch字符串转换为数字
	j=0;
	for(i=10;i>=0;i--)
	{
		if(Pitch_Buf[i]>='0'&&Pitch_Buf[i]<='9')
		{
			APP_Pitch+=(Pitch_Buf[i]-'0')*pow(10,j);
			j++;
		}
		if(Pitch_Buf[0]=='-')
		{
			Pitch_symbel=-1;
		}
	}
	//得到偏移量
	APP_Pitch=Pitch_symbel*APP_Pitch;
	APP_Roll=Roll_symbel*APP_Roll;
	//平滑滤波
	Smoothing_Pitch_Buf[Smoothing_Count]=APP_Pitch;
	Smoothing_Roll_Buf[Smoothing_Count]=APP_Roll;
	Smoothing_Count++;
	//选择值
	if(Smoothing_Count==5)
	{
		Smoothing_Count=0;
		
		for(j = 0; j < 5 - 1; j++) 
		{
        for(i = 0; i < 5 - j; i++) 
				{
            if(Smoothing_Pitch_Buf[i] > Smoothing_Pitch_Buf[i + 1]) 
						{
                Pitch_temp = Smoothing_Pitch_Buf[i];
                Smoothing_Pitch_Buf[i] = Smoothing_Pitch_Buf[i + 1];
                Smoothing_Pitch_Buf[i + 1] = Pitch_temp;
            }
						if(Smoothing_Roll_Buf[i] > Smoothing_Roll_Buf[i + 1]) 
						{
                Roll_temp = Smoothing_Roll_Buf[i];
                Smoothing_Roll_Buf[i] = Smoothing_Roll_Buf[i + 1];
                Smoothing_Roll_Buf[i + 1] = Roll_temp;
            }			
        }
    }
		//平滑滤波
		APP_Pitch=Smoothing_Pitch_Buf[2];
		APP_Roll=Smoothing_Roll_Buf[2];
		
		Map_pitch = Map(APP_Pitch, -90, 90, -127, 127);
		Map_roll = Map(APP_Roll, -90, 90, -127, 127);
					
		pwm1 =  -Map_pitch + Map_roll;
		pwm2 =  -Map_pitch - Map_roll;
		pwm3 =  -Map_pitch + Map_roll;
		pwm4 =  -Map_pitch - Map_roll;
		
		
		pwm1 = Map(pwm1, -127, 127, -499, 499);
		pwm2 = Map(pwm2, -127, 127, -499, 499);
		pwm3 = Map(pwm3, -127, 127, -499, 499);
		pwm4 = Map(pwm4, -127, 127, -499, 499);

		
		
		if (pwm1 < 20 && pwm1 >-20)pwm1 = 0;
		if (pwm2 < 20 && pwm2 >-20)pwm2 = 0;
		if (pwm3 < 20 && pwm3 >-20)pwm3 = 0;
		if (pwm4 < 20 && pwm4 >-20)pwm4 = 0;

		if (pwm1 > 499)pwm1 = 499;
		if (pwm2 > 499)pwm2 = 499;
		if (pwm3 > 499)pwm3 = 499;
		if (pwm4 > 499)pwm4 = 499;
				
		if (pwm1 < -499)pwm1 = -499;
		if (pwm2 < -499)pwm2 = -499;
		if (pwm3 < -499)pwm3 = -499;
		if (pwm4 < -499)pwm4 = -499;
				

		
		if(pwm1>=0)
		{
			TIM_SetCompare4(TIM2,500-pwm1);//L_BIN2:左后轮
			L_BIN2_ON;
			
		}
		else if(pwm1<0)
		{
			pwm1=abs(pwm1);
			TIM_SetCompare4(TIM2,pwm1);//L_BIN2:左后轮
			L_BIN2_OFF;	
		}
				
		if(pwm2>=0)
		{
			TIM_SetCompare3(TIM2,pwm2);//L_AIN2:左前轮
			L_AIN2_OFF;
		}
		else if(pwm2<0)
		{
			pwm2=abs(pwm2);
			TIM_SetCompare3(TIM2,500-pwm2);//L_AIN2:左前轮
			L_AIN2_ON;
		}
					
		if(pwm3>=0)
		{
			TIM_SetCompare1(TIM2,500-pwm3);//R_AIN2:右前轮
			R_AIN2_ON;
		}
		else if(pwm3<0)
		{
			pwm3=abs(pwm3);
			TIM_SetCompare1(TIM2,pwm3);//R_AIN2:右前轮
			R_AIN2_OFF;
		}
					
		if(pwm4>=0)
		{
			TIM_SetCompare2(TIM2,pwm4);//R_BIN2:右后轮
			R_BIN2_OFF;
		}
		else if(pwm4<0)
		{
			pwm4=abs(pwm4);
			TIM_SetCompare2(TIM2,500-pwm4);//R_BIN2:右后轮
			R_BIN2_ON;
		}	

		memset(Smoothing_Pitch_Buf,0,sizeof(Smoothing_Pitch_Buf));
		memset(Smoothing_Roll_Buf,0,sizeof(Smoothing_Roll_Buf));
		delay_ms(1);	
	}
	
	memset(Roll_Buf,0,10);
	memset(Pitch_Buf,0,10);
	
	delay_ms(1);	
}

/**************************************************
函数名称：Control(void)
函数功能：主控制函数，模式切换与调度
输入参数：无
返回参数：无
***************************************************/
void Control(void)
{
	
	NRF24L01_RX_Mode();
	if(NRF24L01_RxPacket(Con_RxBuf)==0&&Con_RxBuf[0]!=' ')
	{
		if(Con_RxBuf[0]=='L'&&Con_RxBuf[1]=='Y')//蓝牙模式
		{
			mode=1;
		}
		else if(Con_RxBuf[0]=='L'&&Con_RxBuf[1]=='X')//无线模式
		{
			mode=2;
		}
		else if(Con_RxBuf[0]>='P'&&Con_RxBuf[3]<='R')//重力模式
		{
			mode=3;
		}
		else if(Con_RxBuf[0]=='B'&&Con_RxBuf[1]=='Z')//避障模式
		{
			mode=4;
		}
		else if(Con_RxBuf[0]=='G'&&Con_RxBuf[1]=='S')//跟随模式
		{
			mode=5;
		}
		else if(Con_RxBuf[0]=='C'&&Con_RxBuf[1]=='D')//RGB灯带模式
		{
			mode=6;
		}
		delay_ms(10);
		//printf("mode=%d\n",mode);
	}
	else
	{
		mode=1;
	}
	delay_ms(10);

	if(mode==1)
	{
		Bluetooth_Mode();
	}
	else if(mode==2)
	{
		while(1)
		{
			Wireless_Mode();//无线模式
			if(NRF_flag==1)
			{
				mode=1;
				break;
			}
		}
	}
	else if(mode==3)
	{
		while(1)
		{
			Gravity_Mode();//重力模式
			if(MPU_flag==1)
			{
				mode=1;
				break;
			}
		}
	}
	else if(mode==4)
	{
		Evadible_Mode();//避障模式
	}
	else if(mode==5)
	{
		Follow_Mode();//跟随模式
	}
	else if(mode==6)
	{
		while(1)
		{
			RGB_Select();//RGB灯带选择
			if(RGB_flag==1)
			{
				mode=1;
				break;
			}
		}
	}
	
	MPU_flag=0;
	NRF_flag=0;
	RGB_flag=0;
	
	RGB_Show();//RGB灯带显示
	
}

