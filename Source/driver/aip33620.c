/* lcd.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com
 * 日期：2023年11月14日
 * LED驱动文件,LED IC aip33620
 */
#include "hal_conf.h"
#include "board.h"
#include "string.h"
#include <stdio.h>
#include "system.h"
#include "crontab.h"
#include "aip33620.h"

#define DISPLAY_MAX_DUTY						250
#define DISPLAY_TIM_PERIOD					100
#define	DISPLAY_GRID_SZIE						5
#define	DISPLAY_SEG_SZIE						10
#define	DISPLAY_RAM_SZIE						(DISPLAY_GRID_SZIE*DISPLAY_SEG_SZIE)

#define DISPLAY_TIM							TIM2
#define DISPLAY_SCLK_CHANNEL					DMA1_Channel2
#define DISPLAY_DIN_CHANNEL					DMA1_Channel4

#define DISPLAY_TIM_SCLK_LOW					DISPLAY_TIM_PERIOD
#define DISPLAY_TIM_SCLK_HIGH					0
#define DISPLAY_TIM_SCLK_CLK					(DISPLAY_TIM_PERIOD>>1)

#define DISPLAY_TIM_DIN_LOW					0
#define DISPLAY_TIM_DIN_HIGH					DISPLAY_TIM_PERIOD

const uint8_t c_number[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
const uint8_t display_level[11]={0x18,0x14,0x1C,0x12,0x1A,0x16,0x1E,0x9E,0x5E,0xDE,0x3E};
static uint8_t display_ram_data[DISPLAY_RAM_SZIE]={0};
static uint8_t display_grid_data[DISPLAY_GRID_SZIE]={0};
static uint16_t display_timer_data[2048]={
DISPLAY_TIM_SCLK_HIGH,
DISPLAY_TIM_DIN_LOW,
DISPLAY_TIM_SCLK_LOW,
DISPLAY_TIM_DIN_LOW,
DISPLAY_TIM_SCLK_CLK,
DISPLAY_TIM_DIN_HIGH,
DISPLAY_TIM_SCLK_CLK,
DISPLAY_TIM_DIN_LOW,
DISPLAY_TIM_SCLK_LOW,
DISPLAY_TIM_DIN_HIGH,
DISPLAY_TIM_SCLK_HIGH,
DISPLAY_TIM_DIN_HIGH};
static uint8_t display_flag=0; 
static uint8_t display_duty=0; 

void display_timer_config(void)
{
	GPIO_InitTypeDef        GPIO_InitStruct;
    TIM_OCInitTypeDef       TIM_OCInitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);

   
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM2, ENABLE);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStruct);
    TIM_TimeBaseStruct.TIM_Prescaler         =0;
    TIM_TimeBaseStruct.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period            = (DISPLAY_TIM_PERIOD-1);
    TIM_TimeBaseStruct.TIM_ClockDivision     = TIM_CKD_DIV4;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(DISPLAY_TIM, &TIM_TimeBaseStruct);

    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState  = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse        = 0;
    TIM_OCInitStruct.TIM_OCPolarity   = TIM_OCPolarity_Low;
    TIM_OCInitStruct.TIM_OCIdleState  = TIM_OCIdleState_Set;


    TIM_OCInitStruct.TIM_Pulse = DISPLAY_TIM_SCLK_HIGH;
    TIM_OC3Init(DISPLAY_TIM, &TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCPolarity   = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_Pulse = DISPLAY_TIM_DIN_HIGH;
	TIM_OC4Init(DISPLAY_TIM, &TIM_OCInitStruct);

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_2);  /* TIM2_CH3 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_2);  /* TIM2_CH4 */

    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
	
	GPIO_InitStruct.GPIO_Pin   = AIP33620_DIN_PIN;
    GPIO_Init(AIP33620_DIN_GPIO, &GPIO_InitStruct);
	AIP33620_DIN_GPIO->BSRR=AIP33620_DIN_PIN;

	GPIO_InitStruct.GPIO_Pin   = AIP33620_SCLK_PIN;
    GPIO_Init(AIP33620_SCLK_GPIO, &GPIO_InitStruct);
	AIP33620_SCLK_GPIO->BSRR=AIP33620_SCLK_PIN;

	TIM_ARRPreloadConfig(DISPLAY_TIM, ENABLE);        
    TIM_Cmd(DISPLAY_TIM, ENABLE);

    TIM_CtrlPWMOutputs(DISPLAY_TIM, ENABLE);
	
	TIM_DMAConfig(DISPLAY_TIM,TIM_DMABase_CCR3,TIM_DMABurstLength_2Bytes);
	TIM_DMACmd(DISPLAY_TIM,TIM_DMA_Update,ENABLE);

	RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA, ENABLE);

	DISPLAY_SCLK_CHANNEL->CCR = 0x00003692;
	DISPLAY_SCLK_CHANNEL->CPAR=(uint32_t)&(DISPLAY_TIM->DMAR);
	DISPLAY_SCLK_CHANNEL->CMAR=(uint32_t)display_timer_data;
	DISPLAY_SCLK_CHANNEL->CNDTR=12;
	

    //DMA_Cmd(DISPLAY_SCLK_CHANNEL, ENABLE);
	DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_ITConfig(DISPLAY_SCLK_CHANNEL, DMA_IT_TC, ENABLE);
	
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x06;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
}
static void display_write_cmd(uint8_t c)
{
	uint16_t i=0;
	uint8_t j=0;
	uint16_t wait=DISPLAY_WAIT;
	
	while((wait--)&&(display_flag==1));
	
	display_timer_data[i++]=DISPLAY_TIM_SCLK_HIGH;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	display_timer_data[i++]=DISPLAY_TIM_SCLK_LOW;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	for(j=0;j<8;j++)
	{
		display_timer_data[i++]=DISPLAY_TIM_SCLK_CLK;
		if(c&0x80)
		{
			display_timer_data[i++]=DISPLAY_TIM_DIN_HIGH;
		}
		else
		{
			display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
		}
		c<<=1;
	}
	display_timer_data[i++]=DISPLAY_TIM_SCLK_LOW;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	display_timer_data[i++]=DISPLAY_TIM_SCLK_HIGH;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	display_timer_data[i++]=DISPLAY_TIM_SCLK_HIGH;
	display_timer_data[i++]=DISPLAY_TIM_DIN_HIGH;
	
	__disable_irq();
	DISPLAY_SCLK_CHANNEL->CCR &= ~DMA_CCR_EN;
	DISPLAY_SCLK_CHANNEL->CPAR=(uint32_t)&(DISPLAY_TIM->DMAR);
	DISPLAY_SCLK_CHANNEL->CMAR=(uint32_t)display_timer_data;
	DISPLAY_SCLK_CHANNEL->CNDTR=i;
	DISPLAY_TIM->CNT=0;
	DISPLAY_SCLK_CHANNEL->CCR |= DMA_CCR_EN;
	__enable_irq(); 
	display_flag=1;
}

static void display_update_ram(uint8_t is_init)
{
	uint16_t i=0;
	uint8_t j=0,k=0;
	uint16_t wait=DISPLAY_WAIT;
	uint8_t c=0xC0;

	while((wait--)&&(display_flag==1));
	
//	if(((AIP33620_DIN_GPIO->IDR&AIP33620_DIN_PIN)==0)||
//		((AIP33620_SCLK_GPIO->IDR&AIP33620_SCLK_PIN)==0))
//	{
//		display_timer_config();
//	}
	
	display_timer_data[i++]=DISPLAY_TIM_SCLK_HIGH;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	display_timer_data[i++]=DISPLAY_TIM_SCLK_LOW;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	for(j=0;j<8;j++)
	{
		display_timer_data[i++]=DISPLAY_TIM_SCLK_CLK;
		if(c&0x80)
		{
			display_timer_data[i++]=DISPLAY_TIM_DIN_HIGH;
		}
		else
		{
			display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
		}
		c<<=1;
	}

	if(is_init)
	{
		for(k=0;k<80;k++)
		{
			c=0;
			for(j=0;j<8;j++)
			{
				display_timer_data[i++]=DISPLAY_TIM_SCLK_CLK;
				if(c&0x80)
				{
					display_timer_data[i++]=DISPLAY_TIM_DIN_HIGH;
				}
				else
				{
					display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
				}
				c<<=1;
			}
		}

	}
	else
	{
		for(k=0;k<DISPLAY_RAM_SZIE;k++)
		{
			c=display_ram_data[k];
			for(j=0;j<8;j++)
			{
				display_timer_data[i++]=DISPLAY_TIM_SCLK_CLK;
				if(c&0x80)
				{
					display_timer_data[i++]=DISPLAY_TIM_DIN_HIGH;
				}
				else
				{
					display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
				}
				c<<=1;
			}
		}
	}

	display_timer_data[i++]=DISPLAY_TIM_SCLK_LOW;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	display_timer_data[i++]=DISPLAY_TIM_SCLK_HIGH;
	display_timer_data[i++]=DISPLAY_TIM_DIN_LOW;
	display_timer_data[i++]=DISPLAY_TIM_SCLK_HIGH;
	display_timer_data[i++]=DISPLAY_TIM_DIN_HIGH;
	
	__disable_irq();
	DISPLAY_SCLK_CHANNEL->CCR &= ~DMA_CCR_EN;
	DISPLAY_SCLK_CHANNEL->CPAR=(uint32_t)&(DISPLAY_TIM->DMAR);
	DISPLAY_SCLK_CHANNEL->CMAR=(uint32_t)display_timer_data;
	DISPLAY_SCLK_CHANNEL->CNDTR=i;
	DISPLAY_TIM->CNT=0;
	DISPLAY_SCLK_CHANNEL->CCR |= DMA_CCR_EN;
	__enable_irq(); 
	display_flag=1;
}

static void display_grid_to_ram(void)
{
	uint8_t i=0,j=0;
	uint8_t c=0;

	for(j=0;j<DISPLAY_GRID_SZIE;j++)
	{
		c=display_grid_data[j];
		for(i=0;i<DISPLAY_SEG_SZIE;i++)
		{
			if(c&0x01)
			{
				if(i<=3)
				{
					display_ram_data[i*DISPLAY_GRID_SZIE+j]=display_duty;
				}
				else
				{
					display_ram_data[(i+2)*DISPLAY_GRID_SZIE+j]=display_duty;
				}
			}
			else
			{
				if(i<=3)
				{
					display_ram_data[i*DISPLAY_GRID_SZIE+j]=0;
				}
				else
				{
					display_ram_data[(i+2)*DISPLAY_GRID_SZIE+j]=0;
				}
			}
			c>>=1;
		}
	}
}



void display_config(void)
{
	display_timer_config();
		
	display_write_cmd(0x1F);		
	display_write_cmd(0x60);	
	display_write_cmd(0x84);
	display_update_ram(1);
	display_write_cmd(0x8C);
	
//	display_grid_data[0]=0xFF;
//	display_grid_data[1]=0xFF;
//	display_grid_data[2]=0xFF;
//	display_grid_data[3]=0xFF;
//	display_grid_data[4]=0xFF;
//	display_duty=DISPLAY_MAX_DUTY;
	display_grid_data[0]=0;
	display_grid_data[1]=0;
	display_grid_data[2]=0;
	display_grid_data[3]=0;
	display_grid_data[4]=0;
	display_duty=0;
	display_grid_to_ram();
	display_update_ram(0);
}

void display_dma_handler(void)
{
	DISPLAY_SCLK_CHANNEL->CCR &= ~DMA_CCR_EN;
	display_flag=0;
}

void display_update_duty(uint8_t duty)
{
	display_duty=(uint16_t)(duty*25)/10;
	display_write_cmd(0x1F);		
	display_write_cmd(0x60);	
	display_write_cmd(0x8C);
	display_grid_to_ram();
	display_update_ram(0);
}

void display_update_set_temperature(int16_t t,uint8_t unit)
{
	if(unit&TEMP_UNIT_TYPE_FAHRNHEIT)
	{
		if(t>MAX_SET_FAHRENHEIT_TEMPERATURE)
		{
			t=MAX_SET_FAHRENHEIT_TEMPERATURE;
		}
		if(t<MIN_SET_FAHRENHEIT_TEMPERATURE)
		{
			t=MIN_SET_FAHRENHEIT_TEMPERATURE;
		}
	}
	else
	{
		if(t>MAX_SET_TEMPERATURE)
		{
			t=MAX_SET_TEMPERATURE;
		}
		if(t<0)
		{
			t=0;
		}
	}
	
	display_grid_data[0]=0x04;
	if(t>=100)
	{
		display_grid_data[1]=c_number[t/100];
		display_grid_data[2]=c_number[(t%100)/10];
	}
	else
	{
		display_grid_data[1]=0;
		if(t>=10)
		{
			display_grid_data[2]=c_number[t/10];
		}
		else
		{
			display_grid_data[2]=0;
		}
	}
	
	display_grid_data[3]=c_number[t%10];
	if(unit&TEMP_UNIT_TYPE_FAHRNHEIT)
	{
		display_grid_data[4]=0x71;
	}
	else
	{
		display_grid_data[4]=0x58;
	}

	display_grid_to_ram();
}

void display_update_current_temperature(int16_t t,uint8_t unit)
{
	if(t>(MAX_TEMPERATURE/100))
	{
		t=MAX_TEMPERATURE/100;
	}

	if(t<0)
	{
		t=0;
	}
	if(unit&TEMP_UNIT_TYPE_FAHRNHEIT)
	{
		t=32+t*9/5;
	}
	
	display_grid_data[0]=0x04;
	if(t>=100)
	{
		display_grid_data[1]=c_number[t/100];
		display_grid_data[2]=c_number[(t%100)/10];
	}
	else
	{
		display_grid_data[1]=0;
		if(t>=10)
		{
			display_grid_data[2]=c_number[t/10];
		}
		else
		{
			display_grid_data[2]=0;
		}
	}
	
	display_grid_data[3]=c_number[t%10];
	if(unit&TEMP_UNIT_TYPE_FAHRNHEIT)
	{
		display_grid_data[4]=0x71;	//F
	}
	else
	{
		display_grid_data[4]=0x58;
	}

	display_grid_to_ram();
}

void display_update_music_type(uint8_t type)
{
	if(type>3)
	{
		type=3;
	}
	if(type==0)
	{
		display_grid_data[0]=0;
		display_grid_data[1]=0;
		display_grid_data[2]=c_number[0];
		display_grid_data[3]=0x71;
		display_grid_data[4]=0x71;
	}
	else
	{
		display_grid_data[0]=0;
		display_grid_data[1]=0x40;
		display_grid_data[2]=c_number[0];
		display_grid_data[3]=c_number[type];
		display_grid_data[4]=0x40;
	}
	display_grid_to_ram();
}



void display_update_temperature_unit(uint8_t unit)
{
	display_grid_data[0]=0x04;
	display_grid_data[1]=0;
	display_grid_data[2]=0;
	display_grid_data[3]=0;
	if(unit&TEMP_UNIT_TYPE_FAHRNHEIT)
	{
		display_grid_data[4]=0x71;
	}
	else
	{
		display_grid_data[4]=0x58;
	}
	display_grid_to_ram();
}

void display_update_error_u01_u02(uint8_t unit)
{
	display_grid_data[0]=0x04;
	display_grid_data[1]=0x40;
	display_grid_data[2]=0x40;
	display_grid_data[3]=0x40;
	if(unit&TEMP_UNIT_TYPE_FAHRNHEIT)
	{
		display_grid_data[4]=0x71;
	}
	else
	{
		display_grid_data[4]=0x58;
	}
	
	display_grid_to_ram();
}

void display_update_error(uint8_t e)
{
	if(e>4)
	{
		e=5;
	}
	display_grid_data[0]=0;
	display_grid_data[1]=0x79;
	display_grid_data[2]=0x40;
	display_grid_data[3]=c_number[0];
	display_grid_data[4]=c_number[e];
	
	display_grid_to_ram();
}

void display_show_pass(void)
{
	display_grid_data[0]=0;
	display_grid_data[1]=0x73;
	display_grid_data[2]=0x77;
	display_grid_data[3]=0x6D;
	display_grid_data[4]=0x6D;
	
	display_grid_to_ram();
}

void display_show_fin(void)
{
	display_grid_data[0]=0;
	display_grid_data[1]=0x71;
	display_grid_data[2]=0x06;
	display_grid_data[3]=0x37;
	display_grid_data[4]=0;
	
	display_grid_to_ram();
}

void display_update_number(uint16_t t)
{
	if(t>9999)
	{
		t=9999;
	}
	display_grid_data[0]=0;

	if(t>=1000)
	{
		display_grid_data[1]=c_number[t/1000];
		t%=1000;
	}
	else
	{
		display_grid_data[1]=0;
	}
	
	if(t>=100)
	{
		display_grid_data[2]=c_number[t/100];
		t%=100;
	}
	else
	{
		display_grid_data[2]=0;
	}
	if(t>=10)
	{
		display_grid_data[3]=c_number[t/10];
		t%=10;
	}
	else
	{
		display_grid_data[3]=0;
	}

	display_grid_data[4]=c_number[t];
	
	display_grid_to_ram();
}

void display_all_on(void)
{
	display_grid_data[0]=0xFF;
	display_grid_data[1]=0xFF;
	display_grid_data[2]=0xFF;
	display_grid_data[3]=0xFF;
	display_grid_data[4]=0xFF;
	
	display_grid_to_ram();
}

