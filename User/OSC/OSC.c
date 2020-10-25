#include "rtthread.h"

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

uint16_t TimePerDiv_Group[] = {2, 5, 10, 20, 50, 100, 200, 500};
uint8_t  TimePerDiv_Nbr = sizeof(TimePerDiv_Group)/sizeof(TimePerDiv_Group[0]);
uint8_t  TimePerDiv_Oder = 0;

//Ҫ��ʾ����Ϣ


FlagStatus Setting=RESET;

volatile   uint16_t    TimePerDiv = 1;//��ʾ���ʱ�䳤��
uint8_t                TriggerMode = 1;//����ģʽ
uint32_t               TriggerValue = 1;//������ƽ
__IO       uint16_t    ADC_ConvertedValue[ADC_SampleNbr] = {0};//ADC�ɼ�����

/* �����߳̿��ƿ� */
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
	LCD_SetColors(WHITE, BLACK);
	ILI9341_Clear(0,0,199,LCD_Y_LENGTH);
	for(i=0; i <= ADC_SampleNbr-2; i++)
	{
		LCD_SetTextColor(WHITE);
		ILI9341_DrawLine ( i, ADC_ConvertedValue[i] /21, i+1, ADC_ConvertedValue[i+1] /21 );
	}	
}



void Run(void)
{
	GetWave_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "GetWave",              /* �߳����� */
                      ADCx_GetWaveData,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      1,                   /* �̵߳����ȼ� */
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

