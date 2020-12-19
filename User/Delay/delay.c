#include "delay.h"
#include "rtthread.h"
#include "stm32f10x.h"


void rt_hw_us_delay(unsigned int us)
{
	rt_uint32_t delta, current_delay, tick;

	//获得延时需要经过的tick数
	tick = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
	//获得当前时间
	delta = SysTick->VAL;
	
	//循环获得当前时间，直到达到指定时间后退出循环
	do
	{
		if(delta > SysTick->VAL)
		{
			current_delay = delta - SysTick->VAL;
		}
		else
		{
			//延时跨越了一次OS tick的边界
			current_delay = SysTick->LOAD + delta - SysTick->VAL;
		}
	}while(current_delay < tick);
}



