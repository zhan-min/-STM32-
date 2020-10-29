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
void EXTI2_IRQnHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		LED1_ON;
		rt_interrupt_enter();
		rt_mq_send(setting_data_queue,
							 &setting_data_set,
							 sizeof(setting_data_set));
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
		if(TimePerDiv_Oder < TimePerDivOder_Nbr-1)
			TimePerDiv_Oder ++;
		else
			TimePerDiv_Oder = 0;
		
		TimePerDiv = TimePerDiv_Group[TimePerDiv_Oder];		
		
		/*使用c标准库把变量转化成字符串*/
		sprintf(dispBuff,"T: %dms",TimePerDiv);
		ILI9341_Clear(200, 0, 320, (((sFONT *)LCD_GetFont())->Height));	
		ILI9341_DispString_EN(210, 0,dispBuff);
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
		if(TimePerDiv_Oder > 0)
			TimePerDiv_Oder --;
		else
			TimePerDiv_Oder = TimePerDivOder_Nbr-1;
		
		TimePerDiv = TimePerDiv_Group[TimePerDiv_Oder];

		/*使用c标准库把变量转化成字符串*/
		sprintf(dispBuff,"T: %dms",TimePerDiv);
		ILI9341_Clear(200, 0, 320, (((sFONT *)LCD_GetFont())->Height));	
		ILI9341_DispString_EN(210, 0,dispBuff);
	}
	EXTI_ClearITPendingBit(EXTI_Line13);
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
