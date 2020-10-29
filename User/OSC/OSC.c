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
*                               ����
******************************************************************
*/
//��Ϣ����
rt_mq_t setting_data_queue = RT_NULL;//��������״̬��־
rt_mq_t getwave_status_queue = RT_NULL;//�ɼ���ɱ�־
rt_mq_t key_scan_queue = RT_NULL;//����ɨ�迪ʼ��־

/* �����߳̿��ƿ� */
static rt_thread_t Setting_thread  = RT_NULL;
static rt_thread_t GetWave_thread  = RT_NULL;
static rt_thread_t PlotWave_thread = RT_NULL;
static rt_thread_t KeyScan_thread  = RT_NULL;


uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDivOder_Nbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
int8_t  TimePerDiv_Oder = 0;//��ǰÿ����ʱ������

//��������
int8_t   TriggerValue = 0;  //����0��������ֵ
int8_t   TriggerMode = 0;   //����1������ģʽ��0���½��ش�����1�������ش���
int8_t   Sampling_mode = 0; //����2������ģʽ��0���Զ���1����ͨ��2������
uint16_t  TimePerDiv = 1;   //����3��ÿ������ʱ����

//Ҫ��ʾ����Ϣ
__IO       uint16_t    ADC_ConvertedValue[ADCx_1_SampleNbr] = {0};//ADC�ɼ�����



/*
*************************************************************************
*                             ��������
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
*                             �̶߳���
*************************************************************************
*/
//������ʾ
void PlotWave(void* parameter)
{
	uint16_t i;
	rt_err_t  recv_statu = RT_EOK;
	uint8_t   flag = 0;//�������ݲɼ���ɱ�־
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


/***ҡ��ʾ��ͼ
       1
      ^
      |
3 <-- 0 --> 4
      |
      v
       2
***/

//����ɹ��������ƹ��
//����
void Setting(void* parameter)
{
	rt_err_t queue_status = RT_EOK;
	uint8_t  setting_data = 5;//�ݴ���Ϣ���е���Ϣ
	int8_t   CurSetItem = 0;
	uint8_t  key_start_scan = 1;
	while(1)
	{
		queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), RT_WAITING_FOREVER);
		if(queue_status == RT_EOK && setting_data == 0)//��������״̬
		{
			LED2_ON;
			setting_data = 5;//ʹsetting_data���ڷ���Чֵ��Χ��Ϊ�˳�������׼��
			while(setting_data != 0)//�ٴΰ���SWʱ�˳�����
			{
				queue_status = rt_mq_send(key_scan_queue, &key_start_scan, sizeof(key_start_scan));//������Ϣ����ʼɨ�����
				queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), 500);//�������޲������˳�����
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
							Setting_do(CurSetItem, -1);//+1��-1��ʾ�Կ���������к��ֲ���
							break;
						}
						case 4:
						{
							Setting_do(CurSetItem, 1);//+1��-1��ʾ�Կ���������к��ֲ���
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


//����ɨ��
void Key_Scan(void* parameter)
{
	rt_err_t queue_status = RT_EOK;
	uint8_t  recv_data = 0;//���ڱ�����յ�����Ϣ
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
			//�����õȴ���������
			rt_kprintf("key data: %d",setting_data);
			rt_mq_send(setting_data_queue, &setting_data, sizeof(setting_data));
		}
	}
}



void Run(void)
{
	/**********������Ϣ����************/
	setting_data_queue = rt_mq_create("setting_data_queue", 1, 10, RT_IPC_FLAG_FIFO);
	getwave_status_queue = rt_mq_create("getwave_status_queue", 1, 1, RT_IPC_FLAG_FIFO);
	key_scan_queue = rt_mq_create("key_scan_queue", 1, 1, RT_IPC_FLAG_FIFO);
	
	/**********�����߳�************/
	Setting_thread = rt_thread_create("Setting", Setting, RT_NULL, 512, 1, 20);
	if (Setting_thread != RT_NULL)
		rt_thread_startup(Setting_thread);
	
	KeyScan_thread = rt_thread_create("KeyScan", Key_Scan, RT_NULL, 512, 2, 20);
	 if (KeyScan_thread != RT_NULL)
		 rt_thread_startup(KeyScan_thread);
	
	GetWave_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "GetWave",              /* �߳����� */
                      Get_Wave_Data,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      3,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
   if (GetWave_thread != RT_NULL) 
        rt_thread_startup(GetWave_thread);
	 
	 PlotWave_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "PlotWave",              /* �߳����� */
                      PlotWave,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      3,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
   if (PlotWave_thread != RT_NULL)
        rt_thread_startup(PlotWave_thread);
}



/* ------------------------------------------end of file---------------------------------------- */

