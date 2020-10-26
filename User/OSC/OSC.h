#ifndef __OSC_H
#define __OSC_H

#include "rtthread.h"
#include "stm32f10x_it.h"

extern rt_mq_t setting_data_queue;
extern rt_mq_t getwave_status_queue;

extern 						 uint16_t     TimePerDiv_Group[];
extern             uint8_t     	TimePerDiv_Nbr;
extern             uint8_t     	TimePerDiv_Oder;

extern volatile    uint16_t     TimePerDiv;
extern             uint8_t      TriggerMode;//触发模式
extern             uint32_t     TriggerValue;//触发电平
extern __IO        uint16_t     ADC_ConvertedValue[];



void PlotWave(void* parameter);
void Init(void);
void Run(void);

#endif /* __OSC_H */

