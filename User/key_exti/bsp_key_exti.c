#include "bsp_key_exti.h"

static void EXTI_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_IntStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	//配置key1
	NVIC_IntStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_IntStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_IntStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_IntStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_IntStruct);
	
	//配置key2
	NVIC_IntStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_IntStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_IntStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_IntStruct);
}


void EXTI_Key_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	
	//配置中断优先级
	EXTI_NVIC_Config();
	
	//初始化GPIO
	RCC_APB2PeriphClockCmd(KEY1_INT_GPIO_CLK, ENABLE); //key1时钟
	RCC_APB2PeriphClockCmd(KEY2_INT_GPIO_CLK, ENABLE); //key2时钟
	
	//初始化key1
	GPIO_InitStruct.GPIO_Pin = KEY1_INT_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY1_INT_GPIO_PORT, &GPIO_InitStruct);
	
	//初始化key2
	GPIO_InitStruct.GPIO_Pin = KEY2_INT_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY2_INT_GPIO_PORT, &GPIO_InitStruct);
	
	//初始化EXTI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;            //key1
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line13;            //key2
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
}
