/* 该文件统一用于存放中断服务函数 */
#include "stm32f10x_it.h"
#include "board.h"
#include "rtthread.h"

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_it.h"
#include "bsp_adc.h"
#include "bsp_ili9341_lcd.h"
#include "OSC.h"
#include "bsp_Ps2.h"
#include "bsp_led.h"



volatile uint32_t Time_us = 0; // us 计时变量

char        dispBuff[100];
uint16_t   ADC_SampleCount=0;
uint8_t    setting_data_set = 0;





/**
  * @brief  SW的中断处理函数，按SW进入设置状态。
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
	uint8_t i=100,j=100,k=100;
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		while(i-- > 0)
		{
			while(j-- > 0)
			{
				while(k-- > 0);
			}
		}
		if (Read_SW_Data() == RESET)
			{
				rt_interrupt_enter();
				rt_mq_send(setting_data_queue, &setting_data_set, sizeof(setting_data_set));
				while(i-- > 0)
				{
					while(j-- > 0);
				}
				rt_interrupt_leave();				
			}
		else
		{
			rt_interrupt_enter();
			if(SamplStatusNrb == 0)
			{
				SamplStatusNrb = 1;
				CurSamplStatus = SamplStatus[SamplStatusNrb];
				Setting_Inf_Update(5);
				
				StopSample = RESET;
				//rt_thread_resume(GetWave_thread);
				LED2_ON;
			}
			else if(SamplStatusNrb == 1)
			{
				SamplStatusNrb = 0;
				CurSamplStatus = SamplStatus[SamplStatusNrb];
				Setting_Inf_Update(5);
				
				StopSample = SET;
				//rt_thread_suspend(GetWave_thread);
				LED2_OFF;
			}
			rt_interrupt_leave();
		}
		EXTI_ClearITPendingBit(EXTI_Line2);
	}	
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
