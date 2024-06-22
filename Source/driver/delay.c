#include "hal_conf.h"

#define USE_SYSTICK_DELAY 				1	 	

#if USE_SYSTICK_DELAY
static __IO uint32_t TimingDelay;

extern uint32_t SystemCoreClock;
extern void crontab_action(void);

void delay_init(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);

    if (SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000))
    {
//        while (1)
//        {
//        }
    }

    NVIC_SetPriority(SysTick_IRQn, 0x0);
}

void SysTick_Handler(void)
{
    if (TimingDelay != 0x00)
    { 
        TimingDelay--;
    }
	crontab_action();
}

void delay_ms(__IO uint32_t nTime)
{ 
    TimingDelay = nTime;
    
    while(TimingDelay != 0);
}
#else

void delay_us(__IO uint32_t nTime)
{		
    u16 i=0;   
    while(nTime--)   
    {   
        i=10;      
        while(i--);       
    }					 
}

void delay_ms(__IO uint32_t nTime)
{	 		  	  
    u16 i=0;   
    while(nTime--)   
    {      
        i=750;   
        while(i--);        
    }	  	    
} 
#endif








































