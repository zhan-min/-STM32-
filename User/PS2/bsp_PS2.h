#ifndef __bsp_PS2_H
#define __bsp_PS2_H

#include "stm32f10x.h"

#define PS2_SW_PIN    GPIO_Pin_2
#define PS2_SW_PORT   GPIOA
#define PS2_SW_CLK    RCC_APB2Periph_GPIOA

#define PS2_X_PIN    GPIO_Pin_14
#define PS2_X_PORT   GPIOB
#define PS2_X_CLK    RCC_APB2Periph_GPIOB

#define PS2_Y_PIN    GPIO_Pin_15
#define PS2_Y_PORT   GPIOB
#define PS2_Y_CLK    RCC_APB2Periph_GPIOB

void    PS2_Key_Config(void);
uint8_t Read_X_Data(void);
uint8_t Read_Y_Data(void);
uint8_t Read_SW_Data(void);

#endif /* __bsp_PS2_H */

