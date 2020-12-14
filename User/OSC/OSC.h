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
extern  int8_t   RangeModeNrb;
extern  int8_t   TriggerModeNrb;
extern  int8_t   SamplingModeNrb;

extern  int16_t   CurTriggerValue;  //����0��������ֵ
extern  char*     CurRangeMode;     //����1������ģʽ��0���Զ���1���ֶ�
extern  char*     CurTriggerMode;   //����2������ģʽ��0���½��ش�����1�������ش�����2���������½��ش���
extern  char*     CurSamplingMode;  //����3������ģʽ��0���Զ���1����ͨ��2������
extern  uint16_t  CurTimePerDiv;    //����4��ÿ������ʱ����

//Ҫ��ʾ����Ϣ
extern __IO        uint16_t     ADC_ConvertedValue[];//ADC�ɼ�����

extern 						 uint16_t     TimePerDiv_Group[];
extern             uint8_t     	TimePerDivOderNbr;
extern             int8_t     	TimePerDivOder;//��ǰÿ����ʱ������
extern             FlagStatus   StopSample;//ֹͣ������־



void PlotBlackground(void);
void PlotWave(void* parameter);
void Init(void);
void Run(void);

#endif /* __OSC_H */

