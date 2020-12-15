#include "OSC.h"
#include "stm32f10x.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_usart.h" 
#include "bsp_adc.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"
#include "bsp_PS2.h"
#include "bsp_led.h"



#define MeasurementRange   3.2

/*
******************************************************************
*                               变量
******************************************************************
*/
//消息队列
rt_mq_t setting_data_queue = RT_NULL;//进入设置状态标志
rt_mq_t getwave_status_queue = RT_NULL;//采集完成标志
rt_mq_t key_scan_queue = RT_NULL;//键盘扫描开始标志

/* 定义线程控制块 */
rt_thread_t Setting_thread  = RT_NULL;
rt_thread_t GetWave_thread  = RT_NULL;
rt_thread_t PlotWave_thread = RT_NULL;
rt_thread_t KeyScan_thread  = RT_NULL;


//可设置项
char*    SamplStatus[] = {"Stop", "Run"};
char*    TriggerMode[] = {"Up", "Down"};
char*    SamplingMode[] = {"A", "N", "S"};
uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};

int8_t   SamplStatusNrb =0;
int8_t   TriggerModeNrb = 0;
int8_t   SamplingModeNrb =0;
uint8_t  TimePerDivOderNbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
int8_t   TimePerDivOder = 0;//当前每格间隔时间的序号


float     CurTriggerValue = 0.0;    //代号0，触发阀值
char*     CurSamplStatus = {"Run"}; //代号1，采样状态，0：停止采样，1：正在采样
char*     CurTriggerMode = {"Up"};  //代号2，触发模式，0：下降沿触发，1：上升沿触发
char*     CurSamplingMode = {"A"};  //代号3，采样模式，0：自动，1：普通，2：单次
uint16_t  CurTimePerDiv = 500;      //代号4，每格代表的时间间隔

//要显示的信息
float WaveFrq = 0.0;//波形频率，单位kHz
__IO  uint16_t    ADC_ConvertedValue[ADCx_1_SampleNbr] = {0};//ADC采集数据
FlagStatus  StopSample = RESET;//停止采样标志



/*
*************************************************************************
*                             辅助函数
*************************************************************************
*/


/**
  * @brief  执行更改设置操作
  * @param  CurSetItem：当前正在设置的参数
	* @param  Operation： 对参数调整的方向
  * @retval None
  */
static void Setting_do(uint8_t CurSetItem, int8_t Operation)
{
	switch(CurSetItem)
	{
		case 0:
		{
			if((Operation > 0) && (CurTriggerValue < MeasurementRange))
				CurTriggerValue += 0.1;
			if((Operation < 0) && (CurTriggerValue > 0.1))
				CurTriggerValue -= 0.1;
			break;
		}
		case 1:
		{
			SamplStatusNrb += Operation;
			if(SamplStatusNrb < 0)
				SamplStatusNrb = 0;
			if(SamplStatusNrb > 1)
				SamplStatusNrb = 1;
			CurSamplStatus = SamplStatus[SamplStatusNrb];
			break;
		}
		case 2:
		{
			TriggerModeNrb += Operation;
			if(TriggerModeNrb < 0)
				TriggerModeNrb = 0;
			if(TriggerModeNrb > 1)
				TriggerModeNrb = 1;
			CurTriggerMode = TriggerMode[TriggerModeNrb];
			break;
		}
		case 3:
		{
			SamplingModeNrb += Operation;
			if(SamplingModeNrb < 0)
				SamplingModeNrb = 0;
			if(SamplingModeNrb > 2)
				SamplingModeNrb = 2;
			CurSamplingMode = SamplingMode[SamplingModeNrb];
			break;
		}
		case 4:
		{
			TimePerDivOder += Operation;
			if(TimePerDivOder < 0)
				TimePerDivOder = 0;
			if(TimePerDivOder > TimePerDivOderNbr-1)
				TimePerDivOder = TimePerDivOderNbr-1;
			CurTimePerDiv = TimePerDiv_Group[TimePerDivOder];
			break;
		}
		default :
			break;
	}
	rt_kprintf("TriggerValue: %.1f\n",CurTriggerValue);
	rt_kprintf("RangeMode: %s\n",CurSamplStatus);
	rt_kprintf("TriggerMode: %s\n",CurTriggerMode);
	rt_kprintf("Sampling_mode: %s\n",CurSamplingMode);
	rt_kprintf("TimePerDiv: %d\n",CurTimePerDiv);
	rt_kprintf("\n");
}


/**
  * @brief  刷新可设置项的显示
  * @param  Operation：当前正在设置的参数
  * @retval None
  */
void Setting_Inf_Update(uint8_t CurSetItem)
{
	char dispBuff[100];
	ILI9341_Clear(240, 0, 20, 240);
	ILI9341_DispString_EN(240, (((sFONT *)LCD_GetFont())->Height)*CurSetItem, "->");
	switch(CurSetItem)
	{
		case 0:
		{
			ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*0, 60, (((sFONT *)LCD_GetFont())->Height));
			/*使用c标准库把变量转化成字符串*/
			sprintf(dispBuff,"%.1f V", CurTriggerValue);
			ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*0, dispBuff);
			break;
		}
		case 1:
		{
			ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*1, 60, (((sFONT *)LCD_GetFont())->Height));
			ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*1, CurSamplStatus);
			break;
		}
		case 2:
		{
			ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*2, 60, (((sFONT *)LCD_GetFont())->Height));
			ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*2, CurTriggerMode);
			break;
		}
		case 3:
		{
			ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*3, 60, (((sFONT *)LCD_GetFont())->Height));
			ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*3, CurSamplingMode);
			break;
		}
		case 4:
		{
			ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*4, 60, (((sFONT *)LCD_GetFont())->Height));
			/*使用c标准库把变量转化成字符串*/
			sprintf(dispBuff,"%d ms", CurTimePerDiv);
			ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*4, dispBuff);
			break;
		}
	}
	
	
}

/**
  * @brief  绘制波形显示区域框和背景
  * @param  无
  * @retval None
  */
void PlotBlackground(void)
{
	uint8_t space=8, length=10;//虚线比例和短横线长度
	
	LCD_SetColors(WHITE, BLACK);			
	
	ILI9341_Clear(Wave_Centor_X-(Wave_Width/2),Wave_Centor_Y-(Wave_Height/2),Wave_Width,Wave_Height);
	
	//画竖线
	ILI9341_DrawLine      (Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2));//左数第1条竖线
	ILI9341_DrawDottedLine(Wave_Centor_X-50,             Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-50,             Wave_Centor_Y+(Wave_Height/2), space);//左数第2条竖线 虚线
	ILI9341_DrawDottedLine(Wave_Centor_X,                Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X,                Wave_Centor_Y+(Wave_Height/2), space);//左数第3条竖线 虚线
	ILI9341_DrawDottedLine(Wave_Centor_X+50,             Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+50,             Wave_Centor_Y+(Wave_Height/2), space);//左数第4条竖线 虚线
	ILI9341_DrawLine      (Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2));//左数第4条竖线
	
	//画横线
	ILI9341_DrawDottedLine(Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y,                 Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y, space);//中间虚线
	//上面的短横线
	ILI9341_DrawLine(Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-(Wave_Width/2)+length, Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-50-length/2,    Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-50+length/2,           Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-length/2,       Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+length/2,              Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+50-length/2,    Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+50+length/2,           Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+(Wave_Width/2)-length, Wave_Centor_Y-(Wave_Height/2));
	//下面的短横线
	ILI9341_DrawLine(Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X-(Wave_Width/2)+length, Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-50-length/2,    Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X-50+length/2,           Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-length/2,       Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X+length/2,              Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+50-length/2,    Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X+50+length/2,           Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X+(Wave_Width/2)-length, Wave_Centor_Y+(Wave_Height/2));
}


/**
  * @brief  计算波形频率
  * @param  ADC_ConvertedValue：采集的波形数据
  * @retval None
  */
void CalculateFrequency(void)
{
	uint8_t i, WaveLenth=0, WaveLenthSumNrb=0, SumNrb;//自动采样模式下对频率求平均值
	uint8_t    ConvertedTriggerValue = CurTriggerValue/3.3*200-0.5;//用于转换触发阀值
	char dispBuff[100];
	
	if(SamplingModeNrb == 0)
		SumNrb = 4;
	else
		SumNrb = 0;
	//计算波长
	if(TriggerModeNrb == 0)
		{
			for(i=0;i < ADCx_1_SampleNbr-1; i++)
			{
				if((ADC_ConvertedValue[i] >= ConvertedTriggerValue) && (ADC_ConvertedValue[i+1] <= ConvertedTriggerValue))
					WaveLenth = i;
				break;
			}			
		}
	else if(TriggerModeNrb == 1)
		{
			for(i=0;i < ADCx_1_SampleNbr-1; i++)
			{
				if((ADC_ConvertedValue[i] <= ConvertedTriggerValue) && (ADC_ConvertedValue[i+1] >= ConvertedTriggerValue))
					WaveLenth = i;
				break;
			}			
		}
		
		
	if(i < ADCx_1_SampleNbr-1)
	{
		WaveLenth += WaveLenth;	
		if(++WaveLenthSumNrb >= SumNrb)
		{
			WaveLenth = WaveLenth>>2;
			//计算频率
			WaveFrq = 1/(((float)WaveLenth)*((float)CurTimePerDiv)/50);//(1/(WaveLenth*CurTimePerDiv/50/1000))*1000 kHz
			ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*5, 60, (((sFONT *)LCD_GetFont())->Height));
			/*使用c标准库把变量转化成字符串*/
			sprintf(dispBuff,"%.1f kHz", WaveFrq);
			ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*5, dispBuff);
		}
		else
			return;
	}
	else
		return;
}


/*
*************************************************************************
*                             线程定义
*************************************************************************
*/
//波形显示
void PlotWave(void* parameter)
{
	uint16_t i;	
	rt_err_t  recv_statu = RT_EOK;
	uint8_t   flag = 0;//波形数据采集完成标志
	while(1)
	{
		recv_statu = rt_mq_recv(getwave_status_queue, &flag, sizeof(flag), RT_WAITING_FOREVER);
		if(recv_statu == RT_EOK && flag == 1)
		{
			//波形显示
			PlotBlackground();
			for(i=0; i <= ADCx_1_SampleNbr-2; i++)
			{
				LCD_SetTextColor(WHITE);
				ILI9341_DrawLine( Wave_Centor_X-(Wave_Width/2)+i, ADC_ConvertedValue[i], Wave_Centor_X-(Wave_Width/2)+i+1, ADC_ConvertedValue[i+1] );
			}
			//频率显示
			CalculateFrequency();//计算和刷新一起			
		}
		flag = 0;
	}
}


/***摇杆示意图
       1
      ^
      |
3 <-- 0 --> 4
      |
      v
       2
***/

//设置
void Setting(void* parameter)
{
	rt_err_t queue_status = RT_EOK;
	uint8_t  setting_data = 5;//暂存消息队列的消息
	uint8_t  CurSetItem = 0;
	uint8_t  key_start_scan = 1;
	while(1)
	{
		queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), RT_WAITING_FOREVER);
		if(queue_status == RT_EOK && setting_data == 0)//进入设置状态
		{			
			LED2_ON;//进入设置状态指示灯
			setting_data = 5;//使setting_data处于非有效值范围，为退出设置做准备
			while(setting_data != 0)//再次按下SW时退出设置
			{
				queue_status = rt_mq_send(key_scan_queue, &key_start_scan, sizeof(key_start_scan));//发送消息，开始扫描键盘
				queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), 5000);//五秒钟无操作则退出设置
				if(queue_status == RT_EOK)
				{
					rt_kprintf("setting_data: %d\n",setting_data);
					switch(setting_data)
					{
						case 1:
						{
							if(CurSetItem > 0)
							{
								CurSetItem--;
								Setting_Inf_Update(CurSetItem);
							}								
							break;
						}
						case 2:
						{							
							if(CurSetItem < 4)
							{
								CurSetItem++;
								Setting_Inf_Update(CurSetItem);
							}								
							break;
						}
						case 3:
						{
							Setting_do(CurSetItem, -1);//+1，-1表示对可设置项进行何种操作
							Setting_Inf_Update(CurSetItem);
							break;
						}
						case 4:
						{
							Setting_do(CurSetItem, 1);//+1，-1表示对可设置项进行何种操作
							Setting_Inf_Update(CurSetItem);
							break;
						}
						default :
							break;
					}
				}
			}
			LED2_OFF;//退出设置状态
		}
	}
}


//键盘扫描
void Key_Scan(void* parameter)
{
	rt_err_t queue_status = RT_EOK;
	uint8_t  recv_data = 0;//用于保存接收到的消息
	uint8_t  setting_data = 5;
	while(1)
	{
		setting_data = 5;
		queue_status = rt_mq_recv(key_scan_queue, &recv_data, sizeof(recv_data), RT_WAITING_FOREVER);
		if(queue_status == RT_EOK && recv_data == 1)
		{
			while(setting_data > 4)
			{
				if(Read_Y_Data() < 5)
				{
					rt_thread_delay(500);
					if(Read_Y_Data() < 5)
						setting_data = 1;
				}				
				else if(Read_Y_Data() > 250)
				{
					rt_thread_delay(500);
					if(Read_Y_Data() > 250)
						setting_data = 2;
				}
				
				if(Read_X_Data() < 5)
				{
					rt_thread_delay(500);
					if(Read_X_Data() < 5)
						setting_data = 3;
				}
				else if(Read_X_Data() > 250)
				{
					rt_thread_delay(500);
					if(Read_X_Data() > 250)
						setting_data = 4;
				}
			}
			rt_kprintf("setting_data: %d\n",setting_data);
			rt_mq_send(setting_data_queue, &setting_data, sizeof(setting_data));
		}
	}
}



void Run(void)
{
	/**********创建消息队列************/
	setting_data_queue = rt_mq_create("setting_data_queue", 1, 10, RT_IPC_FLAG_FIFO);
	getwave_status_queue = rt_mq_create("getwave_status_queue", 1, 1, RT_IPC_FLAG_FIFO);
	key_scan_queue = rt_mq_create("key_scan_queue", 1, 1, RT_IPC_FLAG_FIFO);
	
	/**********创建线程************/
	Setting_thread = rt_thread_create("Setting", Setting, RT_NULL, 512, 2, 20);
	if (Setting_thread != RT_NULL)
		rt_thread_startup(Setting_thread);
	
	KeyScan_thread = rt_thread_create("KeyScan", Key_Scan, RT_NULL, 512, 2, 20);
	 if (KeyScan_thread != RT_NULL)
		 rt_thread_startup(KeyScan_thread);
	
	GetWave_thread =                         /* 线程控制块指针 */
    rt_thread_create( "GetWave",           /* 线程名字 */
                      Get_Wave,       		 /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      3,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
   if (GetWave_thread != RT_NULL) 
        rt_thread_startup(GetWave_thread);
	 
	 PlotWave_thread =                       /* 线程控制块指针 */
    rt_thread_create( "PlotWave",          /* 线程名字 */
                      PlotWave,            /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      3,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
   if (PlotWave_thread != RT_NULL)
        rt_thread_startup(PlotWave_thread);
}



/* ------------------------------------------end of file---------------------------------------- */

