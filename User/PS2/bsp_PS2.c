#include "bsp_PS2.h"
#include "stm32f10x_gpio.h"

static void EXTI_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	//����PS2��SW�ж�
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);	
}

static void PS2_Key_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	
	//�����ж����ȼ�
	EXTI_NVIC_Config();
	
	//��ʼ��GPIO
	RCC_APB2PeriphClockCmd(PS2_SW_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(PS2_X_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(PS2_Y_CLK, ENABLE);
	
	//��ʼ��SW
	GPIO_InitStruct.GPIO_Pin = PS2_SW_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PS2_SW_PORT, &GPIO_InitStruct);
	
	//��ʼ��X
	GPIO_InitStruct.GPIO_Pin = PS2_X_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PS2_X_PORT, &GPIO_InitStruct);
	
	//��ʼ��Y
	GPIO_InitStruct.GPIO_Pin = PS2_Y_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PS2_Y_PORT, &GPIO_InitStruct);
	
	//��ʼ��EXTI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	
	//��ʼ��SW�ж�
	EXTI_InitStruct.EXTI_Line = EXTI_Line2;
	EXTI_InitStruct.EXTI_Mode= EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
}

uint8_t Read_X_Data(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	return GPIO_ReadInputDataBit(PS2_X_PORT, PS2_X_PIN);
}

uint8_t Read_Y_Data(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	return GPIO_ReadInputDataBit(PS2_Y_PORT, PS2_Y_PIN);
}

