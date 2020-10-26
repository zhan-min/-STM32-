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
*                               ����
******************************************************************
*/
//����ϵͳ����

//��������״̬��־
rt_mq_t setting_data_queue = RT_NULL;
rt_mq_t getwave_status_queue = RT_NULL;



uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDiv_Nbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
uint8_t  TimePerDiv_Oder = 0;

//Ҫ��ʾ����Ϣ


volatile   uint16_t    TimePerDiv = 1;//��ʾ���ʱ�䳤��
uint8_t                TriggerMode = 1;//����ģʽ
uint32_t               TriggerValue = 0;//������ƽ
__IO       uint16_t    ADC_ConvertedValue[ADC_SampleNbr] = {0};//ADC�ɼ�����

/* �����߳̿��ƿ� */
static rt_thread_t Setting_thread = RT_NULL;
static rt_thread_t GetWave_thread = RT_NULL;
static rt_thread_t PlotWave_thread = RT_NULL;



/************************* ȫ�ֱ������� ****************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */

/* ��غ궨�� */
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];




/*
*************************************************************************
*                             �̶߳���
*************************************************************************
*/

void PlotWave(void* parameter)
{
	uint16_t i;
	rt_err_t recv_statu = RT_EOK;
	uint8_t   flag = 0;//�������ݲɼ���ɱ�־
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
																500);//�������޲������˳�����
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
	
	GetWave_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "GetWave",              /* �߳����� */
                      ADCx_GetWaveData,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      2,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (GetWave_thread != RT_NULL) 
        rt_thread_startup(GetWave_thread);
	 
	 PlotWave_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "PlotWave",              /* �߳����� */
                      PlotWave,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      2,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (PlotWave_thread != RT_NULL)
        rt_thread_startup(PlotWave_thread);

}



/* ------------------------------------------end of file---------------------------------------- */

