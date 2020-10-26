#ifndef __OSC_H
#define __OSC_H

#include "rtthread.h"
#include "stm32f10x_it.h"

extern rt_mq_t setting_data_queue;

extern 						 uint16_t     TimePerDiv_Group[];
extern             uint8_t     	TimePerDiv_Nbr;
extern             uint8_t     	TimePerDiv_Oder;

extern volatile    uint16_t     TimePerDiv;
extern             uint8_t      TriggerMode;//����ģʽ
extern             uint32_t     TriggerValue;//������ƽ
extern __IO        uint16_t     ADC_ConvertedValue[];



void PlotWave(void* parameter);
void Init(void);
void Run(void);

#endif /* __OSC_H */

