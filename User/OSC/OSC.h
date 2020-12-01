#ifndef __OSC_H
#define __OSC_H

#include "rtthread.h"
#include "stm32f10x_it.h"

//����ϵͳ����
extern rt_mq_t setting_data_queue;
extern rt_mq_t getwave_status_queue;

//�߳�
extern rt_thread_t GetWave_thread;

//��������
extern             uint16_t    TimePerDiv;//ÿ������ʱ����
extern             int8_t      RangeMode;//����ģʽ��0���Զ���1���ֶ�
extern             int8_t      TriggerMode;//����ģʽ��0���½��ش�����1�������ش���
extern             int8_t      TriggerValue;//������ֵ
extern             int8_t      SamplingMode;//����ģʽ��0���Զ���1����ͨ��2������

//Ҫ��ʾ����Ϣ
extern __IO        uint16_t     ADC_ConvertedValue[];//ADC�ɼ�����

extern 						 uint16_t     TimePerDiv_Group[];
extern             uint8_t     	TimePerDivOderNbr;
extern             int8_t     	TimePerDivOder;//��ǰÿ����ʱ������
extern             FlagStatus   StopSample;//ֹͣ������־




void PlotWave(void* parameter);
void Init(void);
void Run(void);

#endif /* __OSC_H */

