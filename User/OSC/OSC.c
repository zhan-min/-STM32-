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
static rt_thread_t Setting_thread  = RT_NULL;
static rt_thread_t GetWave_thread  = RT_NULL;
static rt_thread_t PlotWave_thread = RT_NULL;
static rt_thread_t KeyScan_thread  = RT_NULL;


uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDivOder_Nbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
int8_t  TimePerDiv_Oder = 0;//当前每格间隔时间的序号

//可设置项
int8_t   TriggerValue = 0;  //代号0，触发阀值
int8_t   TriggerMode = 0;   //代号1，触发模式，0：下降沿触发，1：上升沿触发
int8_t   Sampling_mode = 0; //代号2，采样模式，0：自动，1：普通，2：单次
uint16_t  TimePerDiv = 1;   //代号3，每格代表的时间间隔

//要显示的信息
__IO       uint16_t    ADC_ConvertedValue[ADCx_1_SampleNbr] = {0};//ADC采集数据



/*
*************************************************************************
*                             辅助函数
*************************************************************************
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
			TriggerMode += Operation;
			if(TriggerMode < 0)
				TriggerMode = 0;
			if(TriggerMode > 1)
				TriggerMode = 1;
			break;
		}
		case 2:
		{
			Sampling_mode += Operation;
			if(Sampling_mode < 0)
				Sampling_mode = 0;
			if(Sampling_mode > 2)
				Sampling_mode = 2;
			break;
		}
		case 3:
		{
			TimePerDiv += Operation;
			if(TimePerDiv_Oder < 0)
				TimePerDiv_Oder = 0;
			if(TimePerDiv_Oder > TimePerDivOder_Nbr-1)
				TimePerDiv_Oder = TimePerDivOder_Nbr-1;
			break;
		}
		default :
			break;
	}
	rt_kprintf("TriggerValue: %d\n",TriggerValue);
	rt_kprintf("TriggerMode: %d\n",TriggerMode);
	rt_kprintf("Sampling_mode: %d\n",Sampling_mode);
	rt_kprintf("TimePerDiv_Oder: %d\n",TimePerDiv_Oder);
	rt_kprintf("\n");
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
			LCD_SetColors(WHITE, BLACK);
			ILI9341_Clear(0,0,199,LCD_Y_LENGTH);
			for(i=0; i <= ADCx_1_SampleNbr-2; i++)
			{
				LCD_SetTextColor(WHITE);
				ILI9341_DrawLine ( i, ADC_ConvertedValue[i] /21, i+1, ADC_ConvertedValue[i+1] /21 );
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

//待完成工作：绘制光标
//设置
void Setting(void* parameter)
{
	rt_err_t queue_status = RT_EOK;
	uint8_t  setting_data = 5;//暂存消息队列的消息
	int8_t   CurSetItem = 0;
	uint8_t  key_start_scan = 1;
	while(1)
	{
		queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), RT_WAITING_FOREVER);
		if(queue_status == RT_EOK && setting_data == 0)//进入设置状态
		{
			LED2_ON;
			setting_data = 5;//使setting_data处于非有效值范围，为退出设置做准备
			while(setting_data != 0)//再次按下SW时退出设置
			{
				queue_status = rt_mq_send(key_scan_queue, &key_start_scan, sizeof(key_start_scan));//发送消息，开始扫描键盘
				queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), 500);//五秒钟无操作则退出设置
				if(queue_status == RT_EOK)
				{
					switch(setting_data)
					{
						case 1:
						{
							CurSetItem--;
							if(CurSetItem < 0)
								CurSetItem = 0;
							break;
						}
						case 2:
						{
							CurSetItem++;
							if(CurSetItem > 3)
								CurSetItem = 3;
							break;
						}
						case 3:
						{
							Setting_do(CurSetItem, -1);//+1，-1表示对可设置项进行何种操作
							break;
						}
						case 4:
						{
							Setting_do(CurSetItem, 1);//+1，-1表示对可设置项进行何种操作
							break;
						}
						default :
							break;
					}
				}
			}
			LED2_OFF;
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
		queue_status = rt_mq_recv(key_scan_queue, &recv_data, sizeof(recv_data), RT_WAITING_FOREVER);
		if(queue_status == RT_EOK && recv_data == 1)
		{
			setting_data = Read_Y_Data();
			if(setting_data < 5)
				setting_data = 1;
			else if(setting_data > 240)
				setting_data = 2;
			
			setting_data = Read_X_Data();
			if(setting_data < 5)
				setting_data = 3;
			else if(setting_data > 240)
				setting_data = 4;
			//需设置等待按键按下
			rt_kprintf("key data: %d",setting_data);
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
	Setting_thread = rt_thread_create("Setting", Setting, RT_NULL, 512, 1, 20);
	if (Setting_thread != RT_NULL)
		rt_thread_startup(Setting_thread);
	
	KeyScan_thread = rt_thread_create("KeyScan", Key_Scan, RT_NULL, 512, 2, 20);
	 if (KeyScan_thread != RT_NULL)
		 rt_thread_startup(KeyScan_thread);
	
	GetWave_thread =                          /* 线程控制块指针 */
    rt_thread_create( "GetWave",              /* 线程名字 */
                      Get_Wave_Data,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      3,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
   if (GetWave_thread != RT_NULL) 
        rt_thread_startup(GetWave_thread);
	 
	 PlotWave_thread =                          /* 线程控制块指针 */
    rt_thread_create( "PlotWave",              /* 线程名字 */
                      PlotWave,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      3,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
   if (PlotWave_thread != RT_NULL)
        rt_thread_startup(PlotWave_thread);
}



/* ------------------------------------------end of file---------------------------------------- */

