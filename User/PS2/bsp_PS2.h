#ifndef __bsp_PS2_H
#define __bsp_PS2_H

#include "stm32f10x.h"

#define PS2_SW_PIN    GPIO_Pin_2
#define PS2_SW_PORT   GPIOA
#define PS2_SW_CLK    RCC_APB2Periph_GPIOA

#define PS2_X_PIN    GPIO_Pin_5
#define PS2_X_PORT   GPIOB
#define PS2_X_CLK    RCC_APB2Periph_GPIOB

#define PS2_Y_PIN    GPIO_Pin_6
#define PS2_Y_PORT   GPIOB
#define PS2_Y_CLK    RCC_APB2Periph_GPIOB


uint8_t Read_X_Data(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t Read_Y_Data(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif /* __bsp_PS2_H */

