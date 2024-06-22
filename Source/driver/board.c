/* board.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com
 * 日期：2024年2月26日
 * 提供一些f10x CPU用的公用函数接口
 */
#include "hal_conf.h"
#include "board.h"
#include "crontab.h"
#include "aip33620.h"
#include "voice.h"
#include "trace.h"

adc_channel_t act; 

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* Infinite loop */
  while(1)
  {
  }
}

void gpio_config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFG, ENABLE);

	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_20MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_All;;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_20MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
	
	GPIO_InitStruct.GPIO_Pin   = VOICE_RESET_PIN;;
    GPIO_Init(VOICE_RESET_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = TIM14_PERIOD_PIN;
    GPIO_Init(TIM14_PERIOD_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = TEST_PIN;
    GPIO_Init(TEST_GPIO, &GPIO_InitStruct);
	TEST_GPIO->BRR=TEST_PIN;

	GPIO_InitStruct.GPIO_Pin   = TEST1_PIN;
    GPIO_Init(TEST1_GPIO, &GPIO_InitStruct);
	TEST1_GPIO->BRR=TEST1_PIN;

	GPIO_InitStruct.GPIO_Pin   = LED_1_2_PIN;
    GPIO_Init(LED_1_2_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_3_4_PIN;
    GPIO_Init(LED_3_4_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_5_6_PIN;
    GPIO_Init(LED_5_6_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_7_8_PIN;
    GPIO_Init(LED_7_8_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_9_10_PIN;
    GPIO_Init(LED_9_10_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_11_12_PIN;
    GPIO_Init(LED_11_12_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_13_14_PIN;
    GPIO_Init(LED_13_14_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_15_16_PIN;
    GPIO_Init(LED_15_16_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_17_18_PIN;
    GPIO_Init(LED_17_18_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = LED_19_20_PIN;
    GPIO_Init(LED_19_20_GPIO, &GPIO_InitStruct);
	
	
	GPIO_InitStruct.GPIO_Pin   = RELAY_PIN;
    GPIO_Init(RELAY_GPIO, &GPIO_InitStruct);
	RELAY_OFF();
	
	GPIO_InitStruct.GPIO_Pin   = MUSIC_LED_PIN;
    GPIO_Init(MUSIC_LED_GPIO, &GPIO_InitStruct);
	MUSIC_LED_GPIO->BRR= MUSIC_LED_PIN;
	
	GPIO_InitStruct.GPIO_Pin   = BRIGHTNESS_LED_PIN;
    GPIO_Init(BRIGHTNESS_LED_GPIO, &GPIO_InitStruct);
	BRIGHTNESS_LED_GPIO->BRR=BRIGHTNESS_LED_PIN;
	
	GPIO_InitStruct.GPIO_Pin   = KEEP_LED_PIN;
    GPIO_Init(KEEP_LED_GPIO, &GPIO_InitStruct);
	KEEP_LED_GPIO->BRR= KEEP_LED_PIN;
	
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_20MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPU;
	
	
	GPIO_InitStruct.GPIO_Pin   = START_KEY_PIN;
    GPIO_Init(START_KEY_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = KEEP_KEY_PIN;
    GPIO_Init(KEEP_KEY_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = ZERO_CROSS_PIN;
    GPIO_Init(ZERO_CROSS_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = RELAY_FAIL_PIN;
    GPIO_Init(RELAY_FAIL_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_20MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_FLOATING;
	
	GPIO_InitStruct.GPIO_Pin   = ENCODER_A_PIN;
    GPIO_Init(ENCODER_A_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin   = ENCODER_B_PIN;
    GPIO_Init(ENCODER_B_GPIO, &GPIO_InitStruct);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource15);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);
	
	EXTI->IMR|=EXTI_Line6;
	EXTI->RTSR|=EXTI_Line6;
	EXTI->FTSR|=EXTI_Line6;

	/* EXTI Interrupt */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x02;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void adc_config(void)
{
	ADC_InitTypeDef  ADC_InitStruct;
//    DMA_InitTypeDef  DMA_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2ENR_ADC, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);

    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_Resolution         = ADC_Resolution_12b;
    ADC_InitStruct.ADC_PRESCARE           = ADC_PCLK2_PRESCARE_16;
    ADC_InitStruct.ADC_Mode               = ADC_Mode_Continue;
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStruct.ADC_ExternalTrigConv   = ADC1_ExternalTrigConv_T1_CC1;
    ADC_InitStruct.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_Init(ADC1, &ADC_InitStruct);

//    ADC_DMACmd(ADC1, ENABLE);

    ADC_ChannelSampleConfig(ADC1, ADC_Channel_5, ADC_Samctl_240_5);

    ADC_ANY_NUM_Config(ADC1, 1);
    ADC_ANY_CH_Config(ADC1, 0, ADC_Channel_5);
    ADC_ANY_Cmd(ADC1, ENABLE);

    /* PA1(RV1) PA4(RV2) PA5(RV3) */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    ADC_Cmd(ADC1, ENABLE);
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void relay_pwm_config(void)
{
	GPIO_InitTypeDef        GPIO_InitStruct;
    TIM_OCInitTypeDef       TIM_OCInitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;

    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);

   
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM16, ENABLE);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStruct);
    TIM_TimeBaseStruct.TIM_Prescaler         =127;
    TIM_TimeBaseStruct.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period            = 6014;
    TIM_TimeBaseStruct.TIM_ClockDivision     = TIM_CKD_DIV4;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStruct);

    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState  = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse        = 0;
    TIM_OCInitStruct.TIM_OCPolarity   = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OCIdleState  = TIM_OCIdleState_Set;


    TIM_OCInitStruct.TIM_Pulse = (BOIL_TIMER_PREIOD>>1);
    TIM_OC1Init(TIM16, &TIM_OCInitStruct);

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_2);  /* TIM16_CH1 */

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8 ;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    TIM_Cmd(TIM16, ENABLE);

    TIM_CtrlPWMOutputs(TIM16, ENABLE);
	
	HB_PULS_OFF();
}

void timer3_pwm_config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	NVIC_InitTypeDef        NVIC_InitStruct;
	
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);

 
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM3, ENABLE);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStruct);
    TIM_TimeBaseStruct.TIM_Prescaler         =(RCC_Clocks.PCLK1_Frequency / 1000000 - 1);;
    TIM_TimeBaseStruct.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period            = 3500;
    TIM_TimeBaseStruct.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);
	
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_Cmd(TIM3, ENABLE);
}


void system_clock_config(void)
{
	
  
}

/**
  * @brief  Initializes the Global MSP.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
 

}

/**
  * @brief  DeInitializes the Global MSP.
  * @param  None  
  * @retval None
  */
void HAL_MspDeInit(void)
{
  /* NOTE : This function is eventually modified by the user */

}

static void exti_run_encoder(void)
{
	static uint8_t delay=0;
	system_variate_t *sv=system_get_variate();
	uint8_t a=0;

	sv->encoder_idle_delay=0;
	// if((sv->busy_mark&ENCODER_BUSY)==0)
	// {
	// 	delay=0;git
	// }
	sv->busy_mark|=ENCODER_BUSY;

	if(ENCODER_B_GPIO->IDR&ENCODER_B_PIN)
	{
		if(ENCODER_A_GPIO->IDR&ENCODER_A_PIN)
		{
			a=0;
		}
		else
		{
			a=1;
		}
	}
	else
	{
		if(ENCODER_A_GPIO->IDR&ENCODER_A_PIN)
		{
			a=1;
		}
		else
		{
			a=0;
		}
	}

	if(a)
	{
		if(sv->encoder_priority==encoder_music_type)
		{
			delay++;
			if(delay>=2)
			{
				delay=0;
				if(sv->set_music_type<=2)
				{
					sv->set_music_type++;
					if(sv->encoder_voice_delay==0)
					{
						voice_play(sv->set_music_type,voice1);
						sv->encoder_voice_delay=CRONTAB_TIME_40MS;
					}
				}
			}
		}
		else if(sv->encoder_priority==encoder_ring_duty)
		{
			if(sv->set_ring_duty<=2)
			{
				sv->set_ring_duty++;
				if(sv->encoder_voice_delay==0)
				{
					voice_play(sv->set_info.music_type,voice6);;
					sv->encoder_voice_delay=CRONTAB_TIME_40MS;
				}
			}
		}
		else if(sv->encoder_priority==encoder_temperature_unit)
		{
			if(sv->set_temperature_unit!=TEMP_UNIT_TYPE_CELSIYS)
			{
				sv->set_temperature_unit=TEMP_UNIT_TYPE_CELSIYS;
				if(sv->encoder_voice_delay==0)
				{
					voice_play(sv->set_info.music_type,voice6);
					sv->encoder_voice_delay=CRONTAB_TIME_40MS;
				}
			}
		}
		else if(encoder_temperature==sv->encoder_priority)
		{
			if(sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)
			{
				if(sv->set_fahrenheit_temperature!=MAX_SET_FAHRENHEIT_TEMPERATURE)
				{
					if(sv->encoder_voice_delay==0)
					{
						voice_play(sv->set_info.music_type,voice6);
						sv->encoder_voice_delay=CRONTAB_TIME_40MS;
					}
				}
				sv->set_fahrenheit_temperature+=2;
				if(sv->set_fahrenheit_temperature>MAX_SET_FAHRENHEIT_TEMPERATURE)
				{
					sv->set_fahrenheit_temperature=MAX_SET_FAHRENHEIT_TEMPERATURE;
				}
				sv->set_temperature=(sv->set_fahrenheit_temperature-32)*5/9;
			}
			else
			{
				if(sv->set_temperature!=MAX_SET_TEMPERATURE)
				{
					if(sv->encoder_voice_delay==0)
					{
						voice_play(sv->set_info.music_type,voice6);
						sv->encoder_voice_delay=CRONTAB_TIME_40MS;
					}
				}
				sv->set_temperature++;
				if(sv->set_temperature>MAX_SET_TEMPERATURE)
				{
					sv->set_temperature=MAX_SET_TEMPERATURE;
				}
				sv->set_fahrenheit_temperature=sv->set_fahrenheit_temperature*9/5+32;
				if(sv->set_fahrenheit_temperature&0x01)
				{
					sv->set_fahrenheit_temperature++;
				}
			}
		}
		else if(sv->encoder_priority==encoder_fct_cw)
		{
			if(sv->fct_encoder_count!=MAX_FCT_ENCODER_COUNT)
			{
				if(sv->encoder_voice_delay==0)
				{
					voice_play(sv->set_info.music_type,voice6);
					sv->encoder_voice_delay=CRONTAB_TIME_40MS;
				}
			}
			sv->fct_encoder_count++;
			if(sv->fct_encoder_count>=MAX_FCT_ENCODER_COUNT)
			{
				sv->fct_encoder_count=MAX_FCT_ENCODER_COUNT;
			}
		}
	}
	else
	{
		if(sv->encoder_priority==encoder_music_type)
		{
			delay++;
			if(delay>=2)
			{
				delay=0;
				if(sv->set_music_type>0)
				{
					sv->set_music_type--;
					if(sv->encoder_voice_delay==0)
					{
						voice_play(sv->set_music_type,voice1);
						sv->encoder_voice_delay=CRONTAB_TIME_40MS;
					}
				}
			}
			
		}
		else if(sv->encoder_priority==encoder_ring_duty)
		{
			if(sv->set_ring_duty>1)
			{
				sv->set_ring_duty--;
				if(sv->encoder_voice_delay==0)
				{
					voice_play(sv->set_info.music_type,voice6);
					sv->encoder_voice_delay=CRONTAB_TIME_40MS;
				}
			}
			
		}
		else if(sv->encoder_priority==encoder_temperature_unit)
		{
			if(sv->set_temperature_unit!=TEMP_UNIT_TYPE_FAHRNHEIT)
			{
				sv->set_temperature_unit=TEMP_UNIT_TYPE_FAHRNHEIT;
				if(sv->encoder_voice_delay==0)
				{
					voice_play(sv->set_info.music_type,voice6);
					sv->encoder_voice_delay=CRONTAB_TIME_40MS;
				}
			}
		}
		else if(encoder_temperature==sv->encoder_priority)
		{
			if(sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)
			{
				if(sv->set_fahrenheit_temperature>(MIN_SET_FAHRENHEIT_TEMPERATURE+2))
				{
					sv->set_fahrenheit_temperature-=2;
					if(sv->encoder_voice_delay==0)
					{
						voice_play(sv->set_info.music_type,voice6);
						sv->encoder_voice_delay=CRONTAB_TIME_40MS;
					}
				}
				else
				{
					sv->set_fahrenheit_temperature=MIN_SET_FAHRENHEIT_TEMPERATURE;
				}
				sv->set_temperature=(sv->set_fahrenheit_temperature-32)*5/9;
			}
			else
			{
				if(sv->set_temperature>MIN_SET_TEMPERATURE)
				{
					sv->set_temperature--;
					if(sv->encoder_voice_delay==0)
					{
						voice_play(sv->set_info.music_type,voice6);
						sv->encoder_voice_delay=CRONTAB_TIME_40MS;
					}
				}
				sv->set_fahrenheit_temperature=sv->set_fahrenheit_temperature*9/5+32;
				if(sv->set_fahrenheit_temperature&0x01)
				{
					sv->set_fahrenheit_temperature++;
				}
			}
		}
		else if(sv->encoder_priority==encoder_fct_ccw)
		{
			if(sv->fct_encoder_count!=MAX_FCT_ENCODER_COUNT)
			{
				if(sv->encoder_voice_delay==0)
				{
					voice_play(sv->set_info.music_type,voice6);
					sv->encoder_voice_delay=CRONTAB_TIME_40MS;
				}
			}
			sv->fct_encoder_count++;
			if(sv->fct_encoder_count>=MAX_FCT_ENCODER_COUNT)
			{
				sv->fct_encoder_count=MAX_FCT_ENCODER_COUNT;
			}
		}
	}
}

static void exti_run_zero_cross(void)
{
	static uint32_t off_delay=0;
	static uint32_t on_delay=0;
	static uint32_t tick=0;
	system_variate_t *sv=system_get_variate();
	
	sv->ac_lost_tick=0;
	switch(sv->control_mode)
	{
		case control_idle:
			RELAY_OFF();
			HB_PULS_OFF();
			on_delay=0;
			off_delay=0;
			if(sv->boil_mode==boil_on)
			{
				sv->control_mode=control_hb_start;
			}
			break;
		case control_hb_start:
			if(sv->boil_mode==boil_off)
			{
				on_delay=0;
				off_delay++;
				if(off_delay>2)
				{
					HB_PULS_OFF();
					sv->control_mode=control_idle;
					off_delay=0;
				}
			}
			else
			{
				off_delay=0;
				on_delay++;
				if(on_delay>2)
				{
					HB_PULS_ON();
					sv->run_info.relay1_count++;
					sv->control_mode=control_relay_start;
					on_delay=0;
				}
			}
			break;
		case control_relay_start:
			if(sv->boil_mode==boil_off)
			{
				on_delay=0;
				off_delay++;
				if(off_delay>2)
				{
					RELAY_OFF();
					sv->control_mode=control_hb_start;
					off_delay=0;
				}
			}
			else if(sv->boil_mode==boil_on)
			{
				off_delay=0;
				on_delay++;
				if(on_delay>2)
				{
					RELAY_ON();
					tick=sv->tick;
					sv->run_info.relay2_count++;
					sv->control_mode=control_boiling;
					on_delay=0;
				}
			}
			else
			{
				if((sv->tick-tick)>CRONTAB_TIME_1S)
				{
					tick=sv->tick;
					sv->run_info.relay1_time_s++;
				}
			}
			break;
		case control_boiling:
			if((sv->boil_mode==boil_off)||(sv->boil_mode==boil_pause))
			{
				on_delay=0;
				off_delay++;
				if(off_delay>2)
				{
					RELAY_OFF();
					sv->control_mode=control_relay_start;
					off_delay=0;
				}
			}
			else
			{
				if((sv->tick-tick)>CRONTAB_TIME_1S)
				{
					tick=sv->tick;
					sv->run_info.relay2_time_s++;
					sv->run_info.relay1_time_s++;
				}
				off_delay=0;
				RELAY_ON();
			}
			break;
		default:sv->control_mode=control_idle;break;
	}
}

static void exti_check_relay(void)
{
	system_variate_t *sv=system_get_variate();
	
	sv->ac_lost_tick=0;
	if(sv->control_mode==control_boiling)
	{
		if(RELAY_FAIL_GPIO->IDR&RELAY_FAIL_PIN)
		{
			if(sv->relay_error1_delay==0)
			{
				sv->relay_error1_count++;
				TEST_GPIO->ODR^=TEST_PIN;
				sv->relay_error1_delay=CRONTAB_TIME_20MS;
			}
		}
		else
		{
			sv->relay_error1_count=0;
		}
	}
	else
	{
		//继电器没应该一直高电平
		if(RELAY_FAIL_GPIO->IDR&RELAY_FAIL_PIN)
		{
			sv->relay_error2_count=0;
		}
		else
		{
			if(sv->relay_error2_delay==0)
			{
				sv->relay_error2_count++;
				TEST_GPIO->ODR^=TEST_PIN;
				sv->relay_error2_delay=CRONTAB_TIME_20MS;
			}
		}
	}
}

/***********************************************************************************************************************
  * @brief  This function handles TIM3 Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void TIM3_IRQHandler(void)
{
    if (RESET != TIM_GetITStatus(TIM3, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		TIM3->CR1&=~TIM_CR1_CEN;
		if(ZERO_CROSS_GPIO->IDR&ZERO_CROSS_PIN)
		{
			exti_run_zero_cross();
		}
    }
}


/***********************************************************************************************************************
  * @brief  This function handles EXTI4_15 Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void EXTI4_15_IRQHandler(void)
{
	if (RESET != EXTI_GetITStatus(ENCODER_LINE_B))
    {
		EXTI_ClearITPendingBit(ENCODER_LINE_B);
		exti_run_encoder();
    }
	
	
	if (RESET != EXTI_GetITStatus(EXTI_Line6))
    {
		EXTI_ClearITPendingBit(EXTI_Line6);
		
		if(ZERO_CROSS_GPIO->IDR&ZERO_CROSS_PIN)
		{
			TIM3->CNT=0;
			TIM3->CR1|=TIM_CR1_CEN;
		}
		else
		{
			exti_check_relay();
		}
	 }
}


void DMA1_Channel2_3_IRQHandler(void)
{
    if (RESET != (DMA1->ISR&DMA1_IT_TC2))
    {
		 DMA1->IFCR = DMA1_IT_TC2;
		 display_dma_handler();
	}
	
	// if (RESET != (DMA1->ISR&DMA1_IT_TC3))
    // {
	// 	DMA1->IFCR = DMA1_IT_TC3;
	// 	trace_rx_dma_handler();
    // }
}

/***********************************************************************************************************************
  * @brief  This function handles DMA1_Channel4_7 Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void DMA1_Channel4_7_IRQHandler(void)
{
	if (RESET != (DMA1->ISR&DMA1_IT_TC4))
    {
		DMA1->IFCR = DMA1_IT_TC4;
		trace_tx_dma_handler();
		 
    }
	
    if (RESET != (DMA1->ISR&DMA1_IT_TC6))
    {
		DMA1->IFCR = DMA1_IT_TC6;
		voice_dma_handler();
		 
    }
}


/**
  * @brief  trace_printf.
  * @param  None
  * @retval None
  */
__weak void trace_printf(const char *fmt, ...)
{
 
}

__weak void trace_init(void)
{

}

void HardFault_Handler(void)
{
	while(1)
	{
		//GPIO_BOP(BOARD_LED_GPIO)=BOARD_LED_PIN;
	}
}

