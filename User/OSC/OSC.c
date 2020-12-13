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
rt_thread_t Setting_thread  = RT_NULL;
rt_thread_t GetWave_thread  = RT_NULL;
rt_thread_t PlotWave_thread = RT_NULL;
rt_thread_t KeyScan_thread  = RT_NULL;


uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDivOderNbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
int8_t   TimePerDivOder = 0;//��ǰÿ����ʱ������

//��������
int8_t    TriggerValue = 0;  //����0��������ֵ
int8_t    RangeMode = 0;  //����1������ģʽ��0���Զ���1���ֶ�
int8_t    TriggerMode = 0;   //����2������ģʽ��0���½��ش�����1�������ش�����2���������½��ش���
int8_t    SamplingMode = 0;  //����3������ģʽ��0���Զ���1����ͨ��2������
uint16_t  TimePerDiv = 1;   //����4��ÿ������ʱ����

//Ҫ��ʾ����Ϣ
__IO  uint16_t    ADC_ConvertedValue[ADCx_1_SampleNbr] = {0};//ADC�ɼ�����
FlagStatus  StopSample = RESET;//ֹͣ������־



/*
*************************************************************************
*                             ��������
*************************************************************************
*/


/**
  * @brief  ִ�и������ò���
  * @param  CurSetItem����ǰ�������õĲ���
	* @param  Operation�� �Բ��������ķ���
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
  * @brief  ˢ�¿����������ʾ
  * @param  Operation����ǰ�������õĲ���
  * @retval None
  */
void Setting_Inf_Update(uint8_t CurSetItem)
{
	char dispBuff[100];
	
	ILI9341_Clear(240, 0, 80, 240);
	ILI9341_DispString_EN(240, (((sFONT *)LCD_GetFont())->Height)*CurSetItem, "->");
	
	/*ʹ��c��׼��ѱ���ת�����ַ���*/
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
  * @brief  ���Ʋ�����ʾ�����ͱ���
  * @param  ��
  * @retval None
  */
void PlotBlackground(void)
{
	uint8_t space=8, length=10;//���߱����Ͷ̺��߳���
	
	LCD_SetColors(WHITE, BLACK);			
	
	ILI9341_Clear(Wave_Centor_X-(Wave_Width/2),Wave_Centor_Y-(Wave_Height/2),Wave_Width,Wave_Height);
	
	//������
	ILI9341_DrawLine      (Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2));//������1������
	ILI9341_DrawDottedLine(Wave_Centor_X-50,             Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-50,             Wave_Centor_Y+(Wave_Height/2), space);//������2������ ����
	ILI9341_DrawDottedLine(Wave_Centor_X,                Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X,                Wave_Centor_Y+(Wave_Height/2), space);//������3������ ����
	ILI9341_DrawDottedLine(Wave_Centor_X+50,             Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+50,             Wave_Centor_Y+(Wave_Height/2), space);//������4������ ����
	ILI9341_DrawLine      (Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2));//������4������
	
	//������
	ILI9341_DrawDottedLine(Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y,                 Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y, space);//�м�����
	//����Ķ̺���
	ILI9341_DrawLine(Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-(Wave_Width/2)+length, Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-50-length/2,    Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X-50+length/2,           Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-length/2,       Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+length/2,              Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+50-length/2,    Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+50+length/2,           Wave_Centor_Y-(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y-(Wave_Height/2), Wave_Centor_X+(Wave_Width/2)-length, Wave_Centor_Y-(Wave_Height/2));
	//����Ķ̺���
	ILI9341_DrawLine(Wave_Centor_X-(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X-(Wave_Width/2)+length, Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-50-length/2,    Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X-50+length/2,           Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X-length/2,       Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X+length/2,              Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+50-length/2,    Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X+50+length/2,           Wave_Centor_Y+(Wave_Height/2));
	ILI9341_DrawLine(Wave_Centor_X+(Wave_Width/2), Wave_Centor_Y+(Wave_Height/2), Wave_Centor_X+(Wave_Width/2)-length, Wave_Centor_Y+(Wave_Height/2));
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


/***ҡ��ʾ��ͼ
       1
      ^
      |
3 <-- 0 --> 4
      |
      v
       2
***/

//����
void Setting(void* parameter)
{
	rt_err_t queue_status = RT_EOK;
	uint8_t  setting_data = 5;//�ݴ���Ϣ���е���Ϣ
	uint8_t  CurSetItem = 0;
	uint8_t  key_start_scan = 1;
	while(1)
	{
		queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), RT_WAITING_FOREVER);
		if(queue_status == RT_EOK && setting_data == 0)//��������״̬
		{			
			LED2_ON;//��������״ָ̬ʾ��
			setting_data = 5;//ʹsetting_data���ڷ���Чֵ��Χ��Ϊ�˳�������׼��
			while(setting_data != 0)//�ٴΰ���SWʱ�˳�����
			{
				queue_status = rt_mq_send(key_scan_queue, &key_start_scan, sizeof(key_start_scan));//������Ϣ����ʼɨ�����
				queue_status = rt_mq_recv(setting_data_queue, &setting_data, sizeof(setting_data), 5000);//�������޲������˳�����
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
							Setting_do(CurSetItem, -1);//+1��-1��ʾ�Կ���������к��ֲ���
							Setting_Inf_Update(CurSetItem);
							break;
						}
						case 4:
						{
							Setting_do(CurSetItem, 1);//+1��-1��ʾ�Կ���������к��ֲ���
							Setting_Inf_Update(CurSetItem);
							break;
						}
						default :
							break;
					}
				}
			}
			LED2_OFF;//�˳�����״̬
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
	/**********������Ϣ����************/
	setting_data_queue = rt_mq_create("setting_data_queue", 1, 10, RT_IPC_FLAG_FIFO);
	getwave_status_queue = rt_mq_create("getwave_status_queue", 1, 1, RT_IPC_FLAG_FIFO);
	key_scan_queue = rt_mq_create("key_scan_queue", 1, 1, RT_IPC_FLAG_FIFO);
	
	/**********�����߳�************/
	Setting_thread = rt_thread_create("Setting", Setting, RT_NULL, 512, 2, 20);
	if (Setting_thread != RT_NULL)
		rt_thread_startup(Setting_thread);
	
	KeyScan_thread = rt_thread_create("KeyScan", Key_Scan, RT_NULL, 512, 2, 20);
	 if (KeyScan_thread != RT_NULL)
		 rt_thread_startup(KeyScan_thread);
	
	GetWave_thread =                         /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "GetWave",           /* �߳����� */
                      Get_Wave,       		 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      3,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
   if (GetWave_thread != RT_NULL) 
        rt_thread_startup(GetWave_thread);
	 
	 PlotWave_thread =                       /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "PlotWave",          /* �߳����� */
                      PlotWave,            /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      3,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
   if (PlotWave_thread != RT_NULL)
        rt_thread_startup(PlotWave_thread);
}



/* ------------------------------------------end of file---------------------------------------- */

