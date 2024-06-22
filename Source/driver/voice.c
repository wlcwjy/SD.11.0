/* app_main.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com/wlcwjy@163.com
 * 日期：2024年4月15日
 * 提供声音驱动程序
 */
/* Files include */
#include "hal_conf.h"
#include "board.h"
#include <voice.h>
#include "crontab.h"

#define TRACE_CACHE_LENGHT			32
#define VOICE_I2C_ADDRESS      		0xA2
#define VOICE_WAIT		        		5000

#define VOICE_DMA						DMA1
#define VOICE_TX_DMA_CHANNEL      	DMA1_Channel6

static uint8_t voice_send_buffer[TRACE_CACHE_LENGHT];
static char voice_map[3][6]={{0,2,4,6,8},{12,14,16,18,20},{24,26,28,30,32}};
static uint8_t current_channel=0;

void voice_config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef  I2C_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOD, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFG, ENABLE);
	
	SYSCFG->CFGR|=(0x03<<21);
	
	
//	GPIO_InitStruct.GPIO_Pin   = VOICE_RESET_PIN;;
//    GPIO_Init(VOICE_RESET_GPIO, &GPIO_InitStruct);
//	VOICE_RESET_GPIO->BSRR=VOICE_RESET_PIN;

    I2C_DeInit(I2C1);

    I2C_StructInit(&I2C_InitStruct);
    I2C_InitStruct.I2C_Mode       = I2C_Mode_MASTER;
    I2C_InitStruct.I2C_Speed      = I2C_Speed_STANDARD;
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Send7bitAddress(I2C1, VOICE_I2C_ADDRESS);

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_5);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_20MHz;
//  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
//	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;

	VOICE_TX_DMA_CHANNEL->CCR=0x00000292;
	VOICE_TX_DMA_CHANNEL->CPAR=(uint32_t)&(I2C1->DR);
	VOICE_TX_DMA_CHANNEL->CMAR=(uint32_t)voice_send_buffer;
	VOICE_TX_DMA_CHANNEL->CNDTR=TRACE_CACHE_LENGHT;

    DMA_ClearFlag(DMA1_FLAG_TC6);
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel4_7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 4;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
	I2C_DMAConfigure(I2C1, TDMAE_SET);
}

/***********************************************************************************************************************
  * @brief
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void voice_tx_data(uint8_t *buffer, uint8_t length)
{
    uint8_t i = 0;
	uint16_t wait=VOICE_WAIT;

    for (i = 0; i < length; i++)
    {
        I2C_SendData(I2C1, buffer[i]);

		wait=VOICE_WAIT;
        while (RESET == I2C_GetFlagStatus(I2C1, I2C_STATUS_FLAG_TFE))
        {
			if((wait--)==0)
			{
				return;
			}
        }
    }
}

void voice_set_voice(voice_number_e voice)
{
	voice_config();
	
	voice_send_buffer[0]=0x02;
	voice_send_buffer[1]=voice;
	voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
	voice_send_buffer[3]=0;
	voice_send_buffer[4]=0;
	VOICE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
	VOICE_TX_DMA_CHANNEL->CMAR=(uint32_t)voice_send_buffer;
	VOICE_TX_DMA_CHANNEL->CNDTR=5;
	VOICE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
}

void voice_stop_channel(void)
{
	voice_config();
	
	voice_send_buffer[0]=0x02;
	voice_send_buffer[1]=current_channel+11;
	voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
	voice_send_buffer[3]=0;
	voice_send_buffer[4]=0;
	VOICE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
	VOICE_TX_DMA_CHANNEL->CMAR=(uint32_t)voice_send_buffer;
	VOICE_TX_DMA_CHANNEL->CNDTR=5;
	VOICE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
}



void voice_play(uint8_t index,voice_number_e music)
{
	system_variate_t *sv=system_get_variate();
    static uint8_t voice6_index;
	uint32_t tick;
	
	if((index>3))
	{
		return;
	}

	if(music!=voice6)
	{
		voice_stop_channel();
		tick=sv->tick;
		while(CRONTAB_TIME_100MS>(sv->tick-tick));
	}
	
	voice_config();

	if((music==voice1)||(music==voice2))
	{
		current_channel=voice_channel1;
	}

	if((music==voice3)||(music==voice4)||(music==voice7))
	{
		current_channel=voice_channel3;
	}

	if(music==voice5)
	{
		current_channel=voice_channel2;
	}

	if(index==0)
	{
		if((music==voice3)||(music==voice4))
		{
			voice_send_buffer[0]=0x00;
			voice_send_buffer[1]=voice_map[2][music];
			voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
			voice_send_buffer[3]=0;
			voice_send_buffer[4]=0;
			VOICE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
			VOICE_TX_DMA_CHANNEL->CMAR=(uint32_t)voice_send_buffer;
			VOICE_TX_DMA_CHANNEL->CNDTR=5;
			VOICE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
		}
		else if(music==voice7)
		{
			voice_send_buffer[0]=0x00;
			voice_send_buffer[1]=40;
			voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
			voice_send_buffer[3]=0;
			voice_send_buffer[4]=0;
			VOICE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
			VOICE_TX_DMA_CHANNEL->CMAR=(uint32_t)voice_send_buffer;
			VOICE_TX_DMA_CHANNEL->CNDTR=5;
			VOICE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
		}
	}
	else if(index<=3)
	{
		if(music<=voice5)
		{
			voice_send_buffer[0]=0x00;
			voice_send_buffer[1]=voice_map[index-1][music];
			voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
			voice_send_buffer[3]=0;
			voice_send_buffer[4]=0;
		}
		else if(music==voice6)
		{
			voice_send_buffer[0]=0x00;
			if(voice6_index)
			{
				voice_send_buffer[1]=38;
				voice6_index=0;
			}
			else
			{
				voice_send_buffer[1]=36;
				voice6_index=1;
			}
			voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
			voice_send_buffer[3]=0;
			voice_send_buffer[4]=0;
		}
		else if(music==voice7)
		{
			voice_send_buffer[0]=0x00;
			voice_send_buffer[1]=40;
			voice_send_buffer[2]=(0xFF-VOICE_I2C_ADDRESS-voice_send_buffer[0]-voice_send_buffer[1]);
			voice_send_buffer[3]=0;
			voice_send_buffer[4]=0;
		}
		VOICE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
		VOICE_TX_DMA_CHANNEL->CMAR=(uint32_t)voice_send_buffer;
		VOICE_TX_DMA_CHANNEL->CNDTR=5;
		VOICE_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
	}
}


void voice_dma_handler(void)
{
	VOICE_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;  
//	VOICE_RESET_GPIO->BSRR=VOICE_RESET_PIN;
}





