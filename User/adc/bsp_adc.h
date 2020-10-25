#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f10x.h"


//��������
#define  ADC_SampleNbr             200

//ADC GPIO�궨�� ����ʹ�ø�������
#define  ADC_GPIO_APBxClock_FUN    RCC_APB2PeriphClockCmd
#define  ADC_GPIO_CLK              RCC_APB2Periph_GPIOC
#define  ADC_PORT                  GPIOC
#define  ADC_PIN                   GPIO_Pin_1

//ADC���ѡ�� ������ ADC1/2�����ʹ��ADC3���ж���ص�Ҫ�ĳ�ADC3�ģ�ʱ��Ҳ��ͬ
#define  ADC_APBxCLOCK_FUN         RCC_APB2PeriphClockCmd
#define  ADC_x                     ADC2
#define  ADC_CLK                   RCC_APB2Periph_ADC2

//ADCͨ���궨��
#define  ADC_CHANNEL               ADC_Channel_11

//ADC�ж���غ�
#define  ADC_IRQ                   ADC1_2_IRQn
#define  ADC_IRQHandler            ADC1_2_IRQHandler

void ADCx_Init(void);
FlagStatus Get_Trigger_Status(void);
void ADCx_GetWaveData(void);

#endif /* __BSP_ADC_H */

