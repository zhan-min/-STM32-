#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f10x.h"


//采样点数
#define  ADCx_1_SampleNbr             200


// 双模式时，ADC1和ADC2转换的数据都存放在ADC1的数据寄存器，
// ADC1的在低十六位，ADC2的在高十六位
// 双ADC模式的第一个ADC，必须是ADC1
#define    ADCx_1                           ADC1
#define    ADCx_1_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define    ADCx_1_CLK                       RCC_APB2Periph_ADC1

#define    ADCx_1_GPIO_APBxClock_FUN        RCC_APB2PeriphClockCmd
#define    ADCx_1_GPIO_CLK                  RCC_APB2Periph_GPIOC  
#define    ADCx_1_PORT                      GPIOC
#define    ADCx_1_PIN                       GPIO_Pin_1
#define    ADCx_1_CHANNEL                   ADC_Channel_11

// 双ADC模式的第二个ADC，必须是ADC2
#define    ADCx_2                           ADC2
#define    ADCx_2_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define    ADCx_2_CLK                       RCC_APB2Periph_ADC2

#define    ADCx_2_GPIO_APBxClock_FUN        RCC_APB2PeriphClockCmd
#define    ADCx_2_GPIO_CLK                  RCC_APB2Periph_GPIOC  
#define    ADCx_2_PORT                      GPIOC

//采集X的值
#define    ADCx_2_PIN1                       GPIO_Pin_4
#define    ADCx_2_X_CHANNEL                  ADC_Channel_14

//采集Y的值
#define    ADCx_2_PIN2                       GPIO_Pin_5
#define    ADCx_2_Y_CHANNEL                  ADC_Channel_15

#define    NOFADCx_1_CHANEL                        1
#define    NOFADCx_2_CHANEL                        1



void ADCx_Init(void);
void Get_Wave_Data(void* parameter);
uint16_t Get_X_Data(void);
uint16_t Get_Y_Data(void);

#endif /* __BSP_ADC_H */

