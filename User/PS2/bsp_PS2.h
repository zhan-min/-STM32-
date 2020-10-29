#ifndef __bsp_PS2_H
#define __bsp_PS2_H

#include "stm32f10x.h"

#define PS2_SW_PIN    GPIO_Pin_2
#define PS2_SW_PORT   GPIOA
#define PS2_SW_CLK    RCC_APB2Periph_GPIOA

void    PS2_Key_Config(void);
uint8_t Read_X_Data(void);
uint8_t Read_Y_Data(void);
uint8_t Read_SW_Data(void);

#endif /* __bsp_PS2_H */

