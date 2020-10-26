#include "OSC.h"
#include "stm32f10x.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_usart.h" 
#include "bsp_adc.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"
#include "bsp_key_exti.h"


/*
******************************************************************
*                               变量
******************************************************************
*/
//操作系统变量

//进入设置状态标志
rt_mq_t setting_data_queue = RT_NULL;
rt_mq_t getwave_status_queue = RT_NULL;



uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDiv_Nbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
uint8_t  TimePerDiv_Oder = 0;

//要显示的信息


volatile   uint16_t    TimePerDiv = 1;//显示间隔时间长度
uint8_t                TriggerMode = 1;//触发模式
uint32_t               TriggerValue = 0;//触发电平
__IO       uint16_t    ADC_ConvertedValue[ADC_SampleNbr] = {0};//ADC采集数据

/* 定义线程控制块 */
static rt_thread_t Setting_thread = RT_NULL;
static rt_thread_t GetWave_thread = RT_NULL;
static rt_thread_t PlotWave_thread = RT_NULL;



/************************* 全局变量声明 ****************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */

/* 相关宏定义 */
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];




/*
*************************************************************************
*                             线程定义
*************************************************************************
*/

void PlotWave(void* parameter)
{
	uint16_t i;
	rt_err_t recv_statu = RT_EOK;
	uint8_t   flag = 0;//波形数据采集完成标志
	while(1)
	{
		recv_statu = rt_mq_recv(getwave_status_queue, &flag, sizeof(flag), RT_WAITING_FOREVER);
		if(flag == 1)
		{
			LCD_SetColors(WHITE, BLACK);
			ILI9341_Clear(0,0,199,LCD_Y_LENGTH);
			for(i=0; i <= ADC_SampleNbr-2; i++)
			{
				LCD_SetTextColor(WHITE);
				ILI9341_DrawLine ( i, ADC_ConvertedValue[i] /21, i+1, ADC_ConvertedValue[i+1] /21 );
			}
		}
		flag = 0;
	}
}

void Setting(void* parameter)
{
	rt_err_t recv_statu = RT_EOK;
	uint8_t setting_data;
	while(1)
	{
		recv_statu = rt_mq_recv(setting_data_queue,
														&setting_data,
														sizeof(setting_data),
														RT_WAITING_FOREVER);
		if(recv_statu == RT_EOK && setting_data == 0)
		{
			while(setting_data != 1)
			{
				recv_statu = rt_mq_recv(setting_data_queue,
																&setting_data,
																sizeof(setting_data),
																500);//五秒钟无操作则退出设置
				if(recv_statu == RT_EOK)
				{
					switch(setting_data)
					{
						case 2:
						{
							break;
						}
					}
				}
			}
		}
	}
}




void Run(void)
{
	setting_data_queue = rt_mq_create("setting_data_queue",
															1,
															10,
															RT_IPC_FLAG_FIFO);
	getwave_status_queue = rt_mq_create("getwave_status_queue",
															1,
															1,
															RT_IPC_FLAG_FIFO);
	Setting_thread = 
		rt_thread_create("Setting",
											Setting,
											RT_NULL,
											512,
											1,
											20);
	if (Setting_thread != RT_NULL)
		//rt_thread_startup(Setting_thread);
	
	GetWave_thread =                          /* 线程控制块指针 */
    rt_thread_create( "GetWave",              /* 线程名字 */
                      ADCx_GetWaveData,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      2,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (GetWave_thread != RT_NULL) 
        rt_thread_startup(GetWave_thread);
	 
	 PlotWave_thread =                          /* 线程控制块指针 */
    rt_thread_create( "PlotWave",              /* 线程名字 */
                      PlotWave,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      2,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (PlotWave_thread != RT_NULL)
        rt_thread_startup(PlotWave_thread);

}



/* ------------------------------------------end of file---------------------------------------- */

