/* trace.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com
 * 日期：2024年4月8日
 * 提供调试驱动
 */
#include "hal_conf.h"
#include "board.h"
#include <stdarg.h>
#include <stdio.h>
#include "trace.h"
#include <string.h>
#include <flash.h>
#include <crontab.h>
#include <aip33620.h>
#include <voice.h>
#include <led.h>

#define TRACE_QUEUE_MAX_SIZE				10
#define TRACE_CACHE_LENGHT				128

#define TRACE_GPIO							GPIOA


#define TRACE_RX_PIN						GPIO_Pin_10
#define TRACE_TX_PIN						GPIO_Pin_9

#define TRACE_USART						UART1
#define TRACE_DMA							DMA1
#define TRACE_TX_DMA_CHANNEL				DMA1_Channel4
#define TRACE_RX_DMA_CHANNEL				DMA1_Channel3

#define	TRACE_FLAG_TX_DMA_BUSY			(1<<0)
#define	TRACE_FLAG_LOGON					(1<<1)
#define	TRACE_FLAG_DUMP					(1<<2)

typedef struct
{
	uint8_t head;
	uint8_t tail;
	uint8_t trace_size[TRACE_QUEUE_MAX_SIZE];
}trace_queue_t;

static trace_queue_t trace_send_queue;
static trace_queue_t trace_receive_queue;
static char trace_send_buffer[TRACE_QUEUE_MAX_SIZE][TRACE_CACHE_LENGHT];
static char trace_receive_buffer[TRACE_QUEUE_MAX_SIZE][TRACE_CACHE_LENGHT];
static uint8_t trace_flag=0;                     
static uint16_t logon_interval_time=CRONTAB_TIME_1S;   
static uint8_t dump_data[16];
static uint16_t dump_data_count;

const char *ver="ver";
const char *logon="logon";
const char *logoff="logoff";
const char *ul="ul";
const char *clrul="clrul";
const char *pid_duty="change pid duty";
const char *change_pid_duty_off="change pid duty off";
const char *ring_duty="change ring duty";
const char *change_ring_duty_off="change ring duty off";
const char *seg_duty="change segment duty";
const char *change_segment_duty_off="change segment duty off";
const char *indicator_duty="change indicator duty";
const char *change_indicator_duty_off="change indicator duty off";
const char *sound="sound";
const char *ul_dump="ul dump";

void trace_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    UART_InitTypeDef UART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
	
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_UART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA, ENABLE);
	
	SYSCFG->CFGR|=(0x03<<9);
	
    UART_StructInit(&UART_InitStruct);
    UART_InitStruct.BaudRate      = 115200;
    UART_InitStruct.WordLength    = UART_WordLength_8b;
    UART_InitStruct.StopBits      = UART_StopBits_1;
    UART_InitStruct.Parity        = UART_Parity_No;
    UART_InitStruct.HWFlowControl = UART_HWFlowControl_None;
    UART_InitStruct.Mode          = UART_Mode_Tx|UART_Mode_Rx;
	UART_Init(TRACE_USART, &UART_InitStruct);
 
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);

    GPIO_PinAFConfig(TRACE_GPIO, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(TRACE_GPIO, GPIO_PinSource9, GPIO_AF_1);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = TRACE_TX_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(TRACE_GPIO, &GPIO_InitStruct);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = TRACE_RX_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TRACE_GPIO, &GPIO_InitStruct);
	
	UART_ITConfig(TRACE_USART, UART_IT_RXIEN, ENABLE);
	//TRACE_USART->IER |= UART_IER_RXIDLE_IEN;

	UART_Cmd(TRACE_USART, ENABLE);
	
	UART_DMACmd(TRACE_USART, ENABLE);

	trace_receive_queue.head=0;
	trace_receive_queue.tail=0;

	trace_send_queue.head=0;
	trace_send_queue.tail=0;
	TRACE_TX_DMA_CHANNEL->CCR=0x00003292;
	TRACE_TX_DMA_CHANNEL->CPAR=(uint32_t)&(TRACE_USART->TDR);
	TRACE_TX_DMA_CHANNEL->CMAR=(uint32_t)trace_send_buffer[trace_send_queue.head];
	TRACE_TX_DMA_CHANNEL->CNDTR=TRACE_CACHE_LENGHT;

    DMA_ClearFlag(DMA1_FLAG_TC4);
    DMA_ITConfig(TRACE_TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
	
    NVIC_InitStruct.NVIC_IRQChannel = UART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x05;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

static void trcae_send(void)
{
	
	trace_send_queue.head++;
	
	if(trace_send_queue.head>=TRACE_QUEUE_MAX_SIZE)
	{
		trace_send_queue.head=0;
	}
	
	if(trace_flag&TRACE_FLAG_TX_DMA_BUSY)
	{
		return;
	}
	
	trace_flag|=TRACE_FLAG_TX_DMA_BUSY;
	
	TRACE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
	TRACE_TX_DMA_CHANNEL->CMAR=(uint32_t)trace_send_buffer[trace_send_queue.tail];
	TRACE_TX_DMA_CHANNEL->CNDTR=trace_send_queue.trace_size[trace_send_queue.tail];
	TRACE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
	trace_send_queue.tail++;
	if(trace_send_queue.tail>=TRACE_QUEUE_MAX_SIZE)
	{
		trace_send_queue.tail=0;
	}
}

uint16_t trace_rom_crc(uint16_t sum,uint8_t *p,uint32_t len)
{
	static const uint16_t t[]={
		0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70e7,
		0x8108,0x9029,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,	
		};

	while(len--)
	{
		sum=t[(sum>>12)^(*p>>4)]^(sum<<4);
		sum=t[(sum>>12)^(*p++&0xF)]^(sum<<4);
	}
	return sum;
}

void trace_printf(const char *fmt, ...)
{
	va_list args;
	
	if(trace_send_queue.trace_size[trace_send_queue.head]!=0)		//Queue already full 
	{
		return;
	}

	memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
	va_start(args, fmt);
	
	trace_send_queue.trace_size[trace_send_queue.head] = vsnprintf(trace_send_buffer[trace_send_queue.head],TRACE_CACHE_LENGHT, fmt,args);

	va_end(args);	
	
	trace_send_queue.head++;
	
	if(trace_send_queue.head>=TRACE_QUEUE_MAX_SIZE)
	{
		trace_send_queue.head=0;
	}
	memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
	trace_send_queue.trace_size[trace_send_queue.head]=0;

	trcae_send();
}

void trace_handle(void)
{
	static uint32_t tick;
	static uint32_t dump_tick;
	system_variate_t *sv=system_get_variate();
	uint16_t size=trace_receive_queue.trace_size[trace_receive_queue.tail];
	uint32_t rom_check_sum=0;
	char *c;
	uint8_t duty,n;
	uint16_t t;
	
	if(trace_flag&TRACE_FLAG_DUMP)
	{
		if(dump_tick!=sv->tick)
		{
			memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
			flash_epprom_read(dump_data_count,dump_data,16);
			trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],
				"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
				dump_data[0],dump_data[1],dump_data[2],dump_data[3],dump_data[4],dump_data[5],dump_data[6],dump_data[7],
				dump_data[8],dump_data[9],dump_data[10],dump_data[11],dump_data[12],dump_data[13],dump_data[14],dump_data[15]);
			dump_data_count+=16;
			if(dump_data_count>=256)
			{
				trace_flag&=~TRACE_FLAG_DUMP;
			}	
			trcae_send();
			dump_tick=sv->tick;
		}
		return;
	}

	if(trace_flag&TRACE_FLAG_LOGON)
	{	
		if((sv->tick-tick)>=logon_interval_time)
		{
			
			memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
			trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],
			"%d:%02d:%02d:%03d %02d %d %d %d %d\r\n",(sv->tick/360000),((sv->tick%360000)/6000),((sv->tick%6000)/100),((sv->tick%100)*10),
			sv->kettle_mode+2,sv->set_temperature*100,sv->temperature,sv->control_mode==control_boiling?1:2,
			(sv->boil_pid.set_duty==0)?sv->boil_pid.current_duty/15:sv->boil_pid.set_duty/15);
			trcae_send();
			tick=sv->tick;
		}
	}
	
	if(trace_receive_queue.head==trace_receive_queue.tail)
	{
		return;
	}
	
	if(size>=TRACE_CACHE_LENGHT)
	{
		size=(TRACE_CACHE_LENGHT-1);
	}
	
	trace_receive_buffer[trace_receive_queue.tail][size]=0;
	if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],ver))
	{
		rom_check_sum=0;
		rom_check_sum=trace_rom_crc(rom_check_sum,(uint8_t *)(CODE_BASE_ADDRESS),(RUN_INFO_ADDRESS-CODE_BASE_ADDRESS));
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],
			"--- VERSION INFORMATION ---\r\nver:%d.%d\r\nCHECKSUM CRC:%04X\r\n",SW_VERSION,rom_check_sum);
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],logon))
	{
		c=strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],logon);
		n=strlen(logon);
		if(c[n]==' ')
		{
			t=0;
			while(1)
			{
				n++;
				if((c[n]>='0')&&(c[n]<='9'))
				{
					t*=10;
					t+=(c[n]-'0');
				}
				else
				{
					break;
				}
			}
			if((t>=10)&&(t<60000))
			{
				logon_interval_time=t/10;	
				memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
				trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],"logon done:%dms\r\n",t);
				trace_flag|=TRACE_FLAG_LOGON;
				tick=sv->tick;
			}
		}
		else if((c[n]==0x0A)||(c[n]==0x0D))
		{
			logon_interval_time=CRONTAB_TIME_1S;	
			memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
			trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],"logon done:1000ms\r\n");
			trace_flag|=TRACE_FLAG_LOGON;
			tick=sv->tick;
		}
		
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],logoff))
	{
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],"logoff done\r\n");
		trace_flag&=~TRACE_FLAG_LOGON;
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],ul_dump))
	{
		trace_flag|=TRACE_FLAG_DUMP;
		dump_data_count=0;
		dump_tick=sv->tick;
		memset(trace_receive_buffer[trace_receive_queue.tail],0,size);
		trace_receive_queue.trace_size[trace_receive_queue.tail]=0;
		trace_receive_queue.tail++;
		
		if(trace_receive_queue.tail>=TRACE_QUEUE_MAX_SIZE)
		{
			trace_receive_queue.tail=0;
		}
		return;
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],clrul))
	{
		flash_reset_epprom_run_info(&sv->run_info);
		sv->flash_run_time_s=sv->run_info.run_time_s;
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],"clrul done\r\n");
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],ul))
	{
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=sprintf(trace_send_buffer[trace_send_queue.head],
		"Total:%d\r\nSTART/CANCEL:%d\r\nKEEP:%d\r\nRemove Pot:%d\r\nBoiled:%d\r\nKeep Warm:%d\r\nU01:%d\r\nU02:%d\r\nU03:%d\r\nE01:%d\r\nE02(001):%d\r\nE02(002):%d\r\nE03:%d\r\nE04:%d\r\nRelay1:%d\r\nRelay1 Time:%d\r\nRelay2:%d\r\nRelay2 Time:%d\r\nTimeout:%d\r\n",
		sv->run_info.run_time_s,sv->run_info.start_key_count,sv->run_info.keep_key_count,sv->run_info.remove_count,
		sv->run_info.boiled_count,sv->run_info.keep_count,sv->run_info.u01_count,sv->run_info.u02_count,sv->run_info.u03_count,
		sv->run_info.e01_count,sv->run_info.e02_001_count,sv->run_info.e02_002_count,sv->run_info.e03_count,sv->run_info.e04_count,
		sv->run_info.relay1_count,sv->run_info.relay1_time_s,sv->run_info.relay2_count,sv->run_info.relay2_time_s,sv->run_info.plateau_boiling_count);
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],pid_duty))
	{
		c=strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],pid_duty);
		n=strlen(pid_duty);
		if(c[n]==' ')
		{
			n++;
			if((c[n]>='0')&&(c[n]<='9'))
			{
				duty=c[n]-'0';
				n++;
				if((c[n]>='0')&&(c[n]<='9'))
				{
					duty*=10;
					duty+=(c[n]-'0');
					n++;
					if((c[n]>='0')&&(c[n]<='9'))
					{
						duty*=10;
						duty+=(c[n]-'0');
					}
				}
				if(duty<=100)
				{
					sv->boil_pid.set_duty=duty*15;
				}
			}	
		}
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"PID Duty:%d\r\n",duty);
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],change_pid_duty_off))
	{
		sv->boil_pid.set_duty=0;
		sv->boil_mode=boil_off;
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Change pid duty off\r\n");
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],change_ring_duty_off))
	{
		led_set_trace_animation(animation_lighting,0);
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Ring Duty Force Change OFF\r\n");
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],ring_duty))
	{
		c=strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],ring_duty);
		n=strlen(ring_duty);
		if(c[n]==' ')
		{
			n++;
			if((c[n]>='0')&&(c[n]<='9'))
			{
				duty=c[n]-'0';
				n++;
				if((c[n]>='0')&&(c[n]<='9'))
				{
					duty*=10;
					duty+=(c[n]-'0');
					n++;
					if((c[n]>='0')&&(c[n]<='9'))
					{
						duty*=10;
						duty+=(c[n]-'0');
					}
				}
				if(duty<=100)
				{
					led_set_trace_animation(animation_lighting,duty);
				}
			}	
		}
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Ring Duty:%d\r\n",duty);
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],change_segment_duty_off))
	{
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		led_set_trace_animation(animation_nixie_tube,0);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Segment Duty Force Change OFF\r\n");
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],seg_duty))
	{
		c=strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],seg_duty);
		n=strlen(seg_duty);
		if(c[n]==' ')
		{
			n++;
			if((c[n]>='0')&&(c[n]<='9'))
			{
				duty=c[n]-'0';
				n++;
				if((c[n]>='0')&&(c[n]<='9'))
				{
					duty*=10;
					duty+=(c[n]-'0');
					n++;
					if((c[n]>='0')&&(c[n]<='9'))
					{
						duty*=10;
						duty+=(c[n]-'0');
					}
				}
				if(duty<=100)
				{
					led_set_trace_animation(animation_nixie_tube,duty);
				}
			}	
		}
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Segment Duty:%d\r\n",duty);
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],change_indicator_duty_off))
	{
		led_set_trace_animation(animation_music_led,0);
		led_set_trace_animation(animation_brightness_led,0);
		led_set_trace_animation(animation_keep_led,0);
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Indicator Duty Force Change OFF\r\n");
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],indicator_duty))
	{
		c=strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],indicator_duty);
		n=strlen(indicator_duty);
		if(c[n]==' ')
		{
			n++;
			if((c[n]>='0')&&(c[n]<='9'))
			{
				duty=c[n]-'0';
				n++;
				if((c[n]>='0')&&(c[n]<='9'))
				{
					duty*=10;
					duty+=(c[n]-'0');
					n++;
					if((c[n]>='0')&&(c[n]<='9'))
					{
						duty*=10;
						duty+=(c[n]-'0');
					}
				}
				if((duty>0)&&(duty<=100))
				{
					led_set_trace_animation(animation_music_led,duty);
					led_set_trace_animation(animation_brightness_led,duty);
					led_set_trace_animation(animation_keep_led,duty);
				}
			}	
		}
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		trace_send_queue.trace_size[trace_send_queue.head]=
			sprintf(trace_send_buffer[trace_send_queue.head],"Indicator Duty:%d\r\n",duty);
	}
	else if(strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],sound))
	{
		memset(trace_send_buffer[trace_send_queue.head],0,TRACE_CACHE_LENGHT);
		c=strstr((const char *)trace_receive_buffer[trace_receive_queue.tail],sound);
		n=strlen(sound);
		if(c[n]==' ')
		{
			n++;
			if((c[n]<='1')&&(c[n]>='0'))
			{
				duty=(c[n]-'0');
				n++;
				if(duty==1)
				{
					if((c[n]>='0')&&(c[n]<='7'))
					{
						duty=10+(c[n]-'0');
					}
					else
					{
						duty=0;
					}
				}
				else
				{
					if((c[n]>='1')&&(c[n]<='9'))
					{
						duty=(c[n]-'0');
					}
					else
					{
						duty=0;
					}
				}
				if((duty>=1)&&(duty<=5))
				{
					duty--;
					voice_play(1,(voice_number_e)duty);
					trace_send_queue.trace_size[trace_send_queue.head]=
						sprintf(trace_send_buffer[trace_send_queue.head],"sound=A%d\r\n",duty+1);
					
				}
				else if((duty>=6)&&(duty<=10))
				{
					duty-=6;
					voice_play(2,(voice_number_e)duty);
					trace_send_queue.trace_size[trace_send_queue.head]=
						sprintf(trace_send_buffer[trace_send_queue.head],"sound=B%d\r\n",duty+1);
				}
				else if((duty>=11)&&(duty<=15))
				{
					duty-=11;
					voice_play(3,(voice_number_e)duty);
					trace_send_queue.trace_size[trace_send_queue.head]=
						sprintf(trace_send_buffer[trace_send_queue.head],"sound=C%d\r\n",duty+1);
				}
				else if(duty==16)
				{
					voice_play(1,voice6);
					trace_send_queue.trace_size[trace_send_queue.head]=
						sprintf(trace_send_buffer[trace_send_queue.head],"sound=D6\r\n");
				}
				else if(duty==17)
				{
					voice_play(1,voice7);
					trace_send_queue.trace_size[trace_send_queue.head]=
						sprintf(trace_send_buffer[trace_send_queue.head],"sound=D7\r\n");
				}
			}
		}
	}
	else
	{
		memset(trace_receive_buffer[trace_receive_queue.tail],0,TRACE_CACHE_LENGHT);
		trace_receive_queue.trace_size[trace_receive_queue.tail]=0;
		trace_receive_queue.tail++;
	
		if(trace_receive_queue.tail>=TRACE_QUEUE_MAX_SIZE)
		{
			trace_receive_queue.tail=0;
		}
		return;
	}
	
	trcae_send();
	
	memset(trace_receive_buffer[trace_receive_queue.tail],0,TRACE_CACHE_LENGHT);
	trace_receive_queue.trace_size[trace_receive_queue.tail]=0;
	trace_receive_queue.tail++;
	
	if(trace_receive_queue.tail>=TRACE_QUEUE_MAX_SIZE)
	{
		trace_receive_queue.tail=0;
	}
}


/***********************************************************************************************************************
  * @brief  This function handles UART2 Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void UART1_IRQHandler(void)
{
	uint8_t rxdata = 0;
	if (RESET != UART_GetITStatus(TRACE_USART, UART_IT_RXIEN))
    {
        rxdata = UART_ReceiveData(TRACE_USART);
        UART_ClearITPendingBit(TRACE_USART, UART_IT_RXIEN);
        if ((0x0D == rxdata)||(0x0A == rxdata))
        {
			trace_receive_buffer[trace_receive_queue.head][trace_receive_queue.trace_size[trace_receive_queue.head]++]=rxdata;
			trace_receive_buffer[trace_receive_queue.head][trace_receive_queue.trace_size[trace_receive_queue.head]++]=0;
            trace_receive_queue.head++;
			if(trace_receive_queue.head>=TRACE_QUEUE_MAX_SIZE)
			{
				trace_receive_queue.head=0;
			}
        }
		else
		{
			trace_receive_buffer[trace_receive_queue.head][trace_receive_queue.trace_size[trace_receive_queue.head]++]=rxdata;
		}
    }
}

void trace_tx_dma_handler(void)
{
	TRACE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
	if(trace_send_queue.head==trace_send_queue.tail)
	{
		trace_send_queue.trace_size[trace_send_queue.head]=0;
		trace_flag&=~TRACE_FLAG_TX_DMA_BUSY;
		return;
	}
	TRACE_TX_DMA_CHANNEL->CMAR=(uint32_t)trace_send_buffer[trace_send_queue.tail];
	TRACE_TX_DMA_CHANNEL->CNDTR=trace_send_queue.trace_size[trace_send_queue.tail];;
	TRACE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
	trace_send_queue.tail++;
	if(trace_send_queue.tail>=TRACE_QUEUE_MAX_SIZE)
	{
		trace_send_queue.tail=0;
	}      
}
