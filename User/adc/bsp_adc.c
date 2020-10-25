#include "bsp_adc.h"
#include "bsp_TiMbase.h"
#include "OSC.h"

//ADC IO端口初始化
static void ADCx_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//开ADC IO端口时钟
	ADC_GPIO_APBxClock_FUN(ADC_GPIO_CLK, ENABLE);
	
	//ADC IO引脚模式配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);
	
}

//ADC模式配置
static  void ADCx_Mode_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	
	ADC_APBxCLOCK_FUN(ADC_CLK, ENABLE);
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	
	ADC_Init(ADC_x, &ADC_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//12MHz
	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL, 1, ADC_SampleTime_71Cycles5);//转换时间7us
	
	ADC_ITConfig(ADC_x, ADC_IT_EOC, ENABLE);
	ADC_Cmd(ADC_x, ENABLE);
	
  // ADC开始校准
	ADC_StartCalibration(ADC_x);
	// 等待校准完成
	while(ADC_GetCalibrationStatus(ADC_x));
	
	ADC_SoftwareStartConvCmd(ADC_x, ENABLE);
}


//static void ADCx_NVIC_Config(void)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	//优先级分组
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
//	
//	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQ;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//}

void ADCx_Init(void)
{
	//ADCx_NVIC_Config();
	ADCx_GPIO_Config();
	ADCx_Mode_Config();
}


FlagStatus Get_Trigger_Status(void)
{
	uint16_t d0, d1;
	
	while(ADC_GetITStatus(ADC_x, ADC_IT_EOC) == RESET);
	d0 = ADC_GetConversionValue(ADC_x);
	ADC_ClearITPendingBit(ADC_x, ADC_IT_EOC);
	
	while(ADC_GetITStatus(ADC_x, ADC_IT_EOC) == RESET);
	d1 = ADC_GetConversionValue(ADC_x);
	ADC_ClearITPendingBit(ADC_x, ADC_IT_EOC);
	
	if(TriggerMode == 1)
	{
		if(d1 - d0 > TriggerValue)
			return SET;
	}
	else if(TriggerMode == 0)
	{
		if(d0 - d1 > TriggerValue)
			return SET;
	}
	return RESET;
}


void ADCx_GetWaveData(void)
{
	uint16_t  ADC_SampleCount=0;
	
	while(Get_Trigger_Status() == RESET);
	
	while(ADC_SampleCount < ADC_SampleNbr)
	{
		while(ADC_GetITStatus(ADC_x, ADC_IT_EOC) != SET);
		ADC_ConvertedValue[ADC_SampleCount] = ADC_GetConversionValue(ADC_x);
		ADC_ClearITPendingBit(ADC_x, ADC_IT_EOC);
		Delay_us( TimePerDiv*1000/50 -7 );
		ADC_SampleCount++;
	}	
}





