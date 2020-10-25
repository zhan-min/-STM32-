#ifndef __OSC_H
#define __OSC_H

#include "stm32f10x_it.h"


extern 						 uint16_t     TimePerDiv_Group[];
extern             uint8_t     	TimePerDiv_Nbr;
extern             uint8_t     	TimePerDiv_Oder;

extern volatile    uint16_t     TimePerDiv;
extern             uint8_t      TriggerMode;//触发模式
extern             uint32_t     TriggerValue;//触发电平
extern __IO        uint16_t     ADC_ConvertedValue[];

extern             FlagStatus   Setting;


void PlotWave(void);
void Init(void);
void Run(void);

#endif /* __OSC_H */

