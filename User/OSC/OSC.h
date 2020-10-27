#ifndef __OSC_H
#define __OSC_H

#include "rtthread.h"
#include "stm32f10x_it.h"

//操作系统变量
extern rt_mq_t setting_data_queue;
extern rt_mq_t getwave_status_queue;

//可设置项
extern             uint16_t     TimePerDiv;//每格代表的时间间隔
extern             int8_t      TriggerMode;//触发模式，0：下降沿触发，1：上升沿触发
extern             int8_t      TriggerValue;//触发阀值
extern             int8_t      Sampling_mode;//采样模式，0：自动，1：普通，2：单次

//要显示的信息
extern __IO        uint16_t     ADC_ConvertedValue[];//ADC采集数据

extern 						 uint16_t     TimePerDiv_Group[];
extern             uint8_t     	TimePerDivOder_Nbr;
extern             int8_t     	TimePerDiv_Oder;//当前每格间隔时间的序号





void PlotWave(void* parameter);
void Init(void);
void Run(void);

#endif /* __OSC_H */

