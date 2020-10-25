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


uint16_t              ADC_SampleCount=0;


volatile uint32_t Time_us = 0; // us 计时变量



char dispBuff[100];

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

///**
//  * @brief  This function handles Hard Fault exception.
//  * @param  None
//  * @retval None
//  */
//void HardFault_Handler(void)
//{
//  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//  }
//}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

///**
//  * @brief  This function handles PendSVC exception.
//  * @param  None
//  * @retval None
//  */
//void PendSV_Handler(void)
//{
//}

///**
//  * @brief  This function handles SysTick Handler.
//  * @param  None
//  * @retval None
//  */
//void SysTick_Handler(void)
//{
//}




/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/


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
  * @brief  This function handles SW interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQnHandler(void)
{
	if(EXTI_GetFlagStatus(EXTI_Line2) != RESET)
	{
		Setting = SET;
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
		if(TimePerDiv_Oder < TimePerDiv_Nbr-1)
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
			TimePerDiv_Oder = TimePerDiv_Nbr-1;
		
		TimePerDiv = TimePerDiv_Group[TimePerDiv_Oder];

		/*使用c标准库把变量转化成字符串*/
		sprintf(dispBuff,"T: %dms",TimePerDiv);
		ILI9341_Clear(200, 0, 320, (((sFONT *)LCD_GetFont())->Height));	
		ILI9341_DispString_EN(210, 0,dispBuff);
	}
	EXTI_ClearITPendingBit(EXTI_Line13);
}



/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
