#include "OSC.h"
#include "stm32f10x.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_usart.h" 
#include "bsp_adc.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"
#include "bsp_PS2.h"
#include "bsp_led.h"



#define MeasurementRange   30

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


uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDivOderNbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
int8_t   TimePerDivOder = 0;//当前每格间隔时间的序号

//可设置项
int8_t    TriggerValue = 0;  //代号0，触发阀值
int8_t    RangeMode = 0;  //代号1，量程模式，0：自动，1：手动
int8_t    TriggerMode = 0;   //代号2，触发模式，0：下降沿触发，1：上升沿触发，2：上升沿下降沿触发
int8_t    SamplingMode = 0;  //代号3，采样模式，0：自动，1：普通，2：单次
uint16_t  TimePerDiv = 1;   //代号4，每格代表的时间间隔

//要显示的信息
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
			TriggerValue += Operation;
			if(TriggerValue < 0)
				TriggerValue = 0;
			if(TriggerValue > MeasurementRange)
				TriggerValue = MeasurementRange;
			break;
		}
		case 1:
		{
			RangeMode += Operation;
			if(RangeMode < 0)
				RangeMode = 0;
			if(RangeMode > 1)
				RangeMode = 1;
			break;
		}
		case 2:
		{
			TriggerMode += Operation;
			if(TriggerMode < 0)
				TriggerMode = 0;
			if(TriggerMode > 2)
				TriggerMode = 2;
			break;
		}
		case 3:
		{
			SamplingMode += Operation;
			if(SamplingMode < 0)
				SamplingMode = 0;
			if(SamplingMode > 2)
				SamplingMode = 2;
			break;
		}
		case 4:
		{
			TimePerDivOder += Operation;
			if(TimePerDivOder < 0)
				TimePerDivOder = 0;
			if(TimePerDivOder > TimePerDivOderNbr-1)
				TimePerDivOder = TimePerDivOderNbr-1;
			TimePerDiv = TimePerDiv_Group[TimePerDivOder];
			break;
		}
		default :
			break;
	}
	rt_kprintf("TriggerValue: %d\n",TriggerValue);
	rt_kprintf("RangeMode: %d\n",RangeMode);
	rt_kprintf("TriggerMode: %d\n",TriggerMode);
	rt_kprintf("Sampling_mode: %d\n",SamplingMode);
	rt_kprintf("TimePerDiv: %d\n",TimePerDiv);
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
	
	ILI9341_Clear(240, 0, 80, 240);
	ILI9341_DispString_EN(240, (((sFONT *)LCD_GetFont())->Height)*CurSetItem, "->");
	
	/*使用c标准库把变量转化成字符串*/
	sprintf(dispBuff,"TV: %d", TriggerValue);
	ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*0, dispBuff);
	
	sprintf(dispBuff,"RD: %d", RangeMode);
	ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*1, dispBuff);
	
	sprintf(dispBuff,"TM: %d", TriggerMode);
	ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*2, dispBuff);
	
	sprintf(dispBuff,"SM: %d", SamplingMode);
	ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*3, dispBuff);
	
	sprintf(dispBuff,"TPD: %d", TimePerDiv);
	ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*4, dispBuff);
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
			PlotBlackground();
			for(i=0; i <= ADCx_1_SampleNbr-2; i++)
			{
				LCD_SetTextColor(WHITE);
				ILI9341_DrawLine ( Wave_Centor_X-(Wave_Width/2)+i, ADC_ConvertedValue[i] /21, Wave_Centor_X-(Wave_Width/2)+i+1, ADC_ConvertedValue[i+1] /21 );
			}
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

