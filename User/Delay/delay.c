#include "delay.h"
#include "stdint.h"
#include "stm32f10x.h"

static uint8_t fac_us=0;  //us��ʱ������
static uint16_t fac_ms=0; //ms��ʱ������


void DelayInit(void)
{
    //ѡ���ⲿʱ��
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us=SystemCoreClock/8000000;  //����Ϊϵͳʱ�ӵ�1/8
    fac_ms=(u16)fac_us*1000;         
}

void Delay_us(unsigned int us)
{
    u32 temp;
    SysTick->LOAD=us*fac_us;             //����ʱ��
    SysTick->VAL=0x00;                   //��ռ�ʱ��
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp=SysTick->CTRL;
    }
    while(temp&0x01&&!(temp&(1<<16)));         //�ȴ�ʱ�䵽��
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;   //�رռ�ʱ��
    SysTick->VAL=0x00;                         //��ռ�ʱ��
}


void Delay_ms(unsigned int ms)
{
    u32 temp;
    SysTick->LOAD=ms*fac_ms;             
    SysTick->VAL=0x00;                   
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp=SysTick->CTRL;
    }
    while(temp&0x01&&!(temp&(1<<16)));   
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;   
    SysTick->VAL=0x00;                   
}


void Delay_s(unsigned int s)
{
    unsigned char i;
    for(i=0;i<s;i++)
    {
        Delay_ms(1000);
    }


}

