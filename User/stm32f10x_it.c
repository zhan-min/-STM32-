/* 该文件统一用于存放中断服务函数 */
#include "stm32f10x_it.h"
#include "board.h"
#include "rtthread.h"

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_it.h"
#include "bsp_adc.h"
#include "bsp_TiMbase.h"
#include "bsp_ili9341_lcd.h"
#include "OSC.h"
#include <stdio.h>

#include "bsp_led.h"



volatile uint32_t Time_us = 0; // us 计时变量

char        dispBuff[100];
uint16_t   ADC_SampleCount=0;
uint8_t    setting_data_set = 0;


/**
  * @brief  This function handles TIM2 interrupt request.
  * @param  None
  * @retval None
  */
void  BASIC_TIM_IRQHandler (void)
{
	if ( TIM_GetITStatus( BASIC_TIM, TIM_IT_Update) != RESET ) 
	{	
		Time_us++;
		TIM_ClearITPendingBit(BASIC_TIM , TIM_FLAG_Update);  		 
	}		 	
}


/**
  * @brief  SW的中断处理函数，按SW进入设置状态。
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
	uint8_t i=42,j=24, k=92, ClickTime;
	while(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
	ClickTime = 1;
	do
    {
        do
        {
            //再次按下
            if (EXTI_GetITStatus(EXTI_Line2) != RESET)
            {
							ClickTime = 2;
							rt_mq_send(setting_data_queue, &setting_data_set, sizeof(setting_data_set));
            }while (--k);
        } while (--j);
    } while (--i);
	
	if(ClickTime == 1)
	{
		rt_interrupt_enter();
		if(SamplStatusNrb == 0)
		{
			SamplStatusNrb = 1;
			rt_thread_resume(GetWave_thread);
			LED2_ON;
		}
		else if(SamplStatusNrb == 1)
		{
			SamplStatusNrb = 0;
			rt_thread_suspend(GetWave_thread);
			LED2_OFF;
		}
		ILI9341_Clear(260, (((sFONT *)LCD_GetFont())->Height)*6, 60, (((sFONT *)LCD_GetFont())->Height));
		ILI9341_DispString_EN(260, (((sFONT *)LCD_GetFont())->Height)*4, CurSamplStatus);
		rt_interrupt_leave();
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}



/**
  * @brief  This function handles KEY1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		rt_interrupt_enter();
		if(StopSample == SET)
		{
			StopSample = RESET;
			rt_thread_resume(GetWave_thread);
			LED1_OFF;
		}
		else if(StopSample == RESET)
		{
			StopSample = SET;
			rt_thread_suspend(GetWave_thread);
			LED1_ON;
		}		
		rt_interrupt_leave();
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}


/**
  * @brief  This function handles KEY2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		
	}
	EXTI_ClearITPendingBit(EXTI_Line13);
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
