/* app_main.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com/wlcwjy@163.com
 * 日期：2024年4月15日
 * 提供led驱动程序
 */
#include "hal_conf.h"
#include "board.h"
#include "led_animation.h"
#include "led.h"
#include "aip33620.h"
#include "voice.h"
#include "slow_action.h"


#define LED_TIME_30MS						30
#define LED_TIME_75MS						1500
#define LED_TIME_100MS						2000
#define LED_TIME_200MS						4000
#define LED_TIME_300MS						6000
#define LED_TIME_500MS						10000
#define LED_TIME_750MS						15000
#define LED_TIME_1S						20000
#define LED_TIME_1500MS					30000
#define LED_TIME_2S						40000
#define LED_TIME_3S						60000
#define LED_TIME_4S						80000

#define LED_TAPE_LIGHTS_NUMBER			10
#define LED_TAPE_LIGHTS_PERIOD			4850


static unsigned long millis;
static animation_info_t animation_info[animation_type_size]={
{animation_1,animation_1,direction_up,0,0,0,100,0,0,0,LED_TIME_75MS,0,0,led_animation_set_brightness},
{animation_1,animation_1,direction_up,0,0,0,100,0,0,0,LED_TIME_75MS,0,0,display_update_duty},
{animation_1,animation_1,direction_up,0,0,0,100,0,0,0,LED_TIME_75MS,0,0,0},
{animation_1,animation_1,direction_up,0,0,0,100,0,0,0,LED_TIME_75MS,0,0,0},
{animation_1,animation_1,direction_up,0,0,0,100,0,0,0,LED_TIME_75MS,0,0,0},
};
uint8_t duty_data[animation_type_size][LED_MAX_DUTY];


void led_config(void)
{
	
	NVIC_InitTypeDef        NVIC_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);

    RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM14, ENABLE);

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Prescaler         = 0;
    TIM_TimeBaseInitStruct.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period            = LED_TAPE_LIGHTS_PERIOD;
    TIM_TimeBaseInitStruct.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStruct);

    TIM_ClearFlag(TIM14, TIM_FLAG_Update);
    TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = TIM14_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_Cmd(TIM14, ENABLE);

}

unsigned long led_millis(void)
{
	return millis;
}

void led_run_animation(void)
{
	uint8_t i=0;
	system_variate_t *sv=system_get_variate();
	
	for(i=0;i<animation_type_size;i++)
	{
		if(animation_info[i].trace_animation_mode!=animation_1)
		{
			if(animation_info[i].timer_count>LED_TIME_100MS)
			{
				animation_info[i].timer_count=0;
				animation_info[i].duty_count=animation_info[i].trace_duty;
				animation_info[i].duty=animation_info[i].trace_duty;
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
			}
			continue;
		}
		
		
		switch (animation_info[i].animation_mode)
		{
		case animation_12:	
		case animation_1:
			if(animation_info[i].timer_count>LED_TIME_100MS)
			{
				animation_info[i].timer_count=0;
				animation_info[i].duty_count=0;
				animation_info[i].duty=0;
				if(animation_info[i].animation_mode==animation_12)
				{
					break;
				}
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
			}
			break;
		case animation_2:
			if(animation_info[i].timer_count>LED_TIME_100MS)
			{
				animation_info[i].timer_count=0;
				animation_info[i].duty_count=animation_info[i].max_duty;
				animation_info[i].duty=animation_info[i].max_duty;
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
			}
			break;
		case animation_3_to_2:
		case animation_3_to_7:
			if(animation_info[i].timer_count<animation_info[i].base_time)
			{
				break;
			}
			animation_info[i].timer_count=0;
			animation_info[i].duty_count++;
			animation_info[i].duty=slow_ease_in_sine_U0_90(0,animation_info[i].max_duty,animation_info[i].duty_count);
			duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
			if(animation_info[i].update_duty)
			{
				animation_info[i].update_duty(animation_info[i].duty);
			}
			if(animation_info[i].duty_count>=animation_info[i].max_duty)
			{
				animation_info[i].duty_count=animation_info[i].max_duty;
				if(animation_3_to_7==animation_info[i].animation_mode)
				{
					animation_info[i].animation_mode=animation_7;
				}
				else
				{
					animation_info[i].animation_mode=animation_2;
				}
			}
			break;
		case animation_4_to_1:
		case animation_4_to_10:
			if(animation_info[i].timer_count<animation_info[i].base_time)
			{
				break;
			}
			animation_info[i].timer_count=0;
			if(animation_info[i].duty_count>0)
			{
				animation_info[i].duty_count--;
				animation_info[i].duty=slow_ease_out_sine_U90_180(animation_info[i].max_duty,0,animation_info[i].duty_count);
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
			}
			else
			{
				if(animation_info[i].animation_mode==animation_4_to_10)
				{
					animation_info[i].base_time=LED_TIME_2S/animation_info[i].max_duty;
					animation_info[i].direction=direction_up;
					animation_info[i].timer_count=0;	
					animation_info[i].animation_mode=animation_10;

				}
				else
				{
					animation_info[i].animation_mode=animation_1;
				}
			}
			break;
		case animation_5_to_1:
			if(animation_info[i].timer_count<animation_info[i].base_time)
			{
				break;
			}
			animation_info[i].timer_count=0;
			if(animation_info[i].duty_count>0)
			{
				animation_info[i].duty_count--;
				animation_info[i].duty=slow_ease_out_cubic_down(animation_info[i].max_duty,0,animation_info[i].duty_count);
				duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
			}
			else
			{
				animation_info[i].animation_mode=animation_1;
				animation_info[i].timer_count=0;
				animation_info[i].duty_count=0;
				animation_info[i].duty=0;
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
			}
			break;
		case animation_6:
		case animation_10:
		case animation_10_to_1:
		case animation_6_to_1:
		case animation_6_to_2:
			if(animation_info[i].direction==direction_up)
			{
				if(animation_info[i].timer_count<animation_info[i].base_time)
				{
					break;
				}
				animation_info[i].timer_count=0;
				animation_info[i].duty_count++;
				animation_info[i].duty=slow_ease_in_out_sine_U0_180(0,animation_info[i].max_duty,animation_info[i].duty_count);
				duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
				if(animation_info[i].duty_count>=animation_info[i].max_duty)
				{
					if(animation_info[i].animation_mode==animation_6_to_2)
					{
						animation_info[i].animation_mode=animation_2;
					}
					else
					{
						animation_info[i].direction=direction_hold_high;
					}
				}
			}
			else if(animation_info[i].direction==direction_down)
			{
				if(animation_info[i].timer_count<animation_info[i].base_time)
				{
					break;
				}
				animation_info[i].timer_count=0;
				if(animation_info[i].duty_count>0)
				{
					animation_info[i].duty_count--;
					animation_info[i].duty=slow_ease_in_out_sine_U180_360(animation_info[i].max_duty,0,animation_info[i].duty_count);
					duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
					if(animation_info[i].update_duty)
					{
						animation_info[i].update_duty(animation_info[i].duty);
					}
				}
				else
				{
					if((animation_info[i].animation_mode==animation_6_to_1)||
					(animation_info[i].animation_mode==animation_10_to_1))

					{
						animation_info[i].animation_mode=animation_1;
					}
					else
					{
						animation_info[i].direction=direction_up;
					}
				}
			}
			else
			{
				if((animation_info[i].timer_count%100)==0)
				{
					if(animation_info[i].update_duty)
					{
						animation_info[i].update_duty(animation_info[i].duty);
					}
				}
				if((animation_info[i].animation_mode==animation_10)||
					(animation_info[i].animation_mode==animation_10_to_1))
				{
					if(animation_info[i].timer_count>=LED_TIME_1S)
					{
						animation_info[animation_lighting].timer_count=0;
						animation_info[i].direction=direction_down;
					}

				}
				else
				{
					if(animation_info[i].timer_count>=LED_TIME_500MS)
					{
						animation_info[animation_lighting].timer_count=0;
						animation_info[i].direction=direction_down;
					}
				}
			}
			break;
		case animation_7:
			led_animation_loop();
			break;
		case animation_8_to_2:
			if(animation_info[i].timer_count<animation_info[i].base_time)
			{
				break;
			}
			animation_info[i].timer_count=0;
			animation_info[i].duty_count++;
			animation_info[i].duty=slow_ease_in_out_sine_U0_180(0,animation_info[i].max_duty,animation_info[i].duty_count);
			duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
			if(animation_info[i].update_duty)
			{
				animation_info[i].update_duty(animation_info[i].duty);
			}
			if(animation_info[i].duty_count>=animation_info[i].max_duty)
			{
				animation_info[i].animation_mode=animation_2;
			}
			break;
		case animation_9:
		case animation_9_to_5:
			if(animation_info[i].direction==direction_up)
			{
				if(animation_info[i].timer_count<animation_info[i].base_time)
				{
					break;
				}
				animation_info[i].timer_count=0;
				animation_info[i].duty_count++;
				animation_info[i].duty=slow_ease_out_cubic_up(0,animation_info[i].max_duty,animation_info[i].duty_count);
				duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
				if(animation_info[i].update_duty)
				{
					animation_info[i].update_duty(animation_info[i].duty);
				}
				if(animation_info[i].duty_count>=animation_info[i].max_duty)
				{
					animation_info[i].animation_mode=animation_5_to_1;
					animation_info[i].base_time=LED_TIME_3S/animation_info[i].max_duty;
					animation_info[i].direction=direction_down;
				}
			}
			else if(animation_info[i].direction==direction_down)
			{
				if(animation_info[i].timer_count<animation_info[i].base_time)
				{
					break;
				}
				animation_info[i].timer_count=0;
				if(animation_info[i].duty_count>0)
				{
					animation_info[i].duty_count--;
					animation_info[i].duty=slow_ease_out_circ(animation_info[i].max_duty,0,animation_info[i].duty_count);
					duty_data[i][animation_info[i].duty_count]=animation_info[i].duty;
					if(animation_info[i].update_duty)
					{
						animation_info[i].update_duty(animation_info[i].duty);
					}
				}
				else
				{
					voice_play(sv->set_info.music_type,voice4);
					animation_info[i].direction=direction_up;
				}
			}
			break;
		case animation_11:
		case animation_11_to_1:
		case animation_11_to_2:
			if(animation_info[i].timer_count>LED_TIME_100MS)
			{
				animation_info[i].timer_count=0;
				animation_info[i].run_times++;
				if(animation_info[i].run_times<=6)
				{
					animation_info[i].duty_count=0;
					animation_info[i].duty=0;
					if(animation_info[i].update_duty)
					{
						animation_info[i].update_duty(animation_info[i].duty);
					}
				}
				else if(animation_info[i].run_times<=12)
				{
					animation_info[i].duty_count=0;
					animation_info[i].duty=animation_info[i].max_duty;
					if(animation_info[i].update_duty)
					{
						animation_info[i].update_duty(animation_info[i].duty);
					}
				}
				else if(animation_info[i].run_times<=18)
				{
					if(animation_info[i].animation_mode==animation_11_to_2)
					{
						animation_info[i].animation_mode=animation_2;
					}
					else
					{
						animation_info[i].duty_count=0;
						animation_info[i].duty=0;
						if(animation_info[i].update_duty)
						{
							animation_info[i].update_duty(animation_info[i].duty);
						}
					}
				}
				else if(animation_info[i].run_times<=24)
				{
					animation_info[i].duty_count=0;
					animation_info[i].duty=animation_info[i].max_duty;
					if(animation_info[i].update_duty)
					{
						animation_info[i].update_duty(animation_info[i].duty);
					}
				}
				else if(animation_info[i].run_times<=30)
				{
					if(animation_info[i].animation_mode==animation_11_to_2)
					{
						animation_info[i].animation_mode=animation_2;
					}
					else
					{
						animation_info[i].duty_count=0;
						animation_info[i].duty=0;
						if(animation_info[i].update_duty)
						{
							animation_info[i].update_duty(animation_info[i].duty);
						}
					}
				}
				else
				{
					animation_info[i].run_times=0;
					if(animation_info[i].animation_mode==animation_11_to_1)
					{
						animation_info[i].animation_mode=animation_1;
					}
				}
			}
			break;
		case animation_13:
			if(i==animation_lighting)
			{
				led_animation13_loop(&animation_info[i]);
			}
			else
			{
				led_set_animation((animation_type_e)i,animation_1,animation_info[i].max_duty);
			}
			break;
		default:
			break;
		}	
	}
}

void led_set_animation(animation_type_e type,animation_mode_e mode,uint16_t max_duty)
{
	system_variate_t *sv=system_get_variate();
	uint16_t i=0;
	uint16_t duty=0;

	if(max_duty>LED_MAX_DUTY)
	{
		animation_info[type].max_duty=LED_MAX_DUTY;
	}
	animation_info[type].max_duty=max_duty;
	
	
	if(mode==animation_6)
	{
		if((animation_info[type].animation_mode!=animation_6)&&
			(animation_info[type].animation_mode!=animation_6_to_1)&&
			(animation_info[type].animation_mode!=animation_6_to_2))
		{
			animation_info[type].base_time=LED_TIME_750MS/max_duty;
			animation_info[type].direction=direction_up;
			animation_info[type].timer_count=0;
		}
	}
	
	if(mode==animation_6_to_1)
	{
		if((animation_info[type].animation_mode!=animation_6)&&
			(animation_info[type].animation_mode!=animation_6_to_1)&&
			(animation_info[type].animation_mode!=animation_6_to_2))
		{
			animation_info[type].base_time=LED_TIME_750MS/max_duty;
			animation_info[type].direction=direction_up;
			animation_info[type].timer_count=0;
		}
	}
	
	if(mode==animation_6_to_2)
	{
		if((animation_info[type].animation_mode!=animation_6)&&
			(animation_info[type].animation_mode!=animation_6_to_1)&&
			(animation_info[type].animation_mode!=animation_6_to_2))
		{
			animation_info[type].base_time=LED_TIME_750MS/max_duty;
			animation_info[type].direction=direction_up;
			animation_info[type].timer_count=0;
		}
	}

	if(animation_10==mode)
	{
		if((animation_info[type].animation_mode!=animation_10)&&
			(animation_info[type].animation_mode!=animation_10_to_1))
		{
			animation_info[type].base_time=LED_TIME_2S/max_duty;
			animation_info[type].direction=direction_up;
			animation_info[type].timer_count=0;	
		}
	}
	
	if(animation_10_to_1==mode)
	{
		if((animation_info[type].animation_mode!=animation_10)&&
			(animation_info[type].animation_mode!=animation_10_to_1))
		{
			animation_info[type].base_time=LED_TIME_2S/max_duty;
			animation_info[type].direction=direction_up;
			animation_info[type].timer_count=0;	
		}
	}
	
	if((animation_info[type].animation_mode!=animation_3_to_2)&&
		(animation_3_to_2==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].direction=direction_up;
		animation_info[type].timer_count=0;
	}
	if((animation_info[type].animation_mode!=animation_4_to_1)&&
		(animation_4_to_1==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].direction=direction_down;
		animation_info[type].timer_count=0;
	}
	if((animation_info[type].animation_mode!=animation_4_to_10)&&
		(animation_4_to_10==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].direction=direction_down;
		animation_info[type].timer_count=0;
	}
	if((animation_info[type].animation_mode!=animation_5_to_1)&&
		(animation_5_to_1==mode))
	{
		animation_info[type].duty_count=animation_info[type].max_duty;
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].direction=direction_down;
		animation_info[type].timer_count=0;	
	}
	if((animation_info[type].animation_mode!=animation_9_to_5)&&
		(animation_9_to_5==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].direction=direction_down;
		animation_info[type].timer_count=0;	
	}
	
	if((animation_info[type].animation_mode!=animation_11_to_1)&&
		(animation_11_to_1==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].timer_count=0;	
		animation_info[type].run_times=0;
	}

	if((animation_info[type].animation_mode!=animation_11_to_2)&&
		(animation_11_to_2==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].timer_count=0;	
		animation_info[type].run_times=0;
	}

	if((animation_info[type].animation_mode!=animation_11)&&
		(animation_11==mode))
	{
		animation_info[type].base_time=LED_TIME_1S/max_duty;
		animation_info[type].timer_count=0;	
		animation_info[type].run_times=0;
	}
	if((animation_info[type].animation_mode!=animation_8_to_2)&&
		(animation_8_to_2==mode))
	{
		animation_info[type].duty=led_animation_get_duty();
		for(animation_info[type].duty_count=0;animation_info[type].duty_count<max_duty;animation_info[type].duty_count++)
		{
			duty=slow_ease_in_out_sine_U0_180(0,max_duty,animation_info[type].duty_count);
			if(duty>animation_info[type].duty)
			{
				break;
			}
		}
		animation_info[type].base_time=LED_TIME_1500MS/(max_duty-animation_info[i].duty_count);
		animation_info[type].timer_count=0;	
		animation_info[type].run_times=0;
	}
	if((animation_info[type].animation_mode!=animation_13)&&(animation_13==mode))
	{
		if(type==animation_lighting)
		{
			animation_info[type].base_time=LED_TIME_200MS/max_duty;
			animation_info[type].timer_count=0;	
			animation_info[type].run_times=0;
			animation_info[type].step=0;
			animation_info[type].duty_count=0;
			animation_info[type].duty_count1=0;
			animation_info[type].duty_count2=0;
		}
		else
		{
			return;
		}
	}
//	if(mode!=animation_1)
//	{
//		animation_info[type].animation_mode=animation_2;
//		return;
//	}
	animation_info[type].animation_mode=mode;
}

void led_set_trace_animation(animation_type_e type,uint16_t max_duty)
{
	animation_info[type].trace_animation_mode=animation_1;
	animation_info[type].trace_duty=max_duty;
	// animation_info[i].timer_count=0;
	// animation_info[i].duty_count=animation_info[i].trace_duty;
	// animation_info[i].duty=animation_info[i].trace_duty;
	// if(animation_info[i].update_duty)
	// {
	// 	animation_info[i].update_duty(animation_info[i].duty);
	// }
}

animation_mode_e led_get_animation(animation_type_e type)
{
	return animation_info[type].animation_mode;
}

/***********************************************************************************************************************
  * @brief  This function handles TIM14 Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void TIM14_IRQHandler(void)
{
	static uint8_t duty_count=0;
	static uint8_t i=0;
	static uint8_t millis_count=0;
	system_variate_t *sv=system_get_variate();
	
    if (RESET != TIM_GetITStatus(TIM14, TIM_IT_Update))
    {
		TIM14_PERIOD_GPIO->BSRR=TIM14_PERIOD_PIN;
        CLEAR_BIT(TIM14->SR, TIM_IT_Update);
		
		for(i=0;i<animation_type_size;i++)
		{
			animation_info[i].timer_count++;
		}
		duty_count++;
		if(duty_count>=100)
		{
			duty_count=0;
		}
		
		millis_count++;
		if(millis_count>=20)
		{
			millis_count=0;
			millis++;
		}
		
		if(duty_count>=animation_info[animation_music_led].duty)
		{
			MUSIC_LED_GPIO->BRR=MUSIC_LED_PIN;
		}
		else
		{
			MUSIC_LED_GPIO->BSRR=MUSIC_LED_PIN;
		}
	
		if(duty_count>=animation_info[animation_brightness_led].duty)
		{
			BRIGHTNESS_LED_GPIO->BRR=BRIGHTNESS_LED_PIN;
		}
		else
		{
			BRIGHTNESS_LED_GPIO->BSRR=BRIGHTNESS_LED_PIN;
		}
		
		if(duty_count>=animation_info[animation_keep_led].duty)
		{
			KEEP_LED_GPIO->BRR=KEEP_LED_PIN;
		}
		else
		{
			KEEP_LED_GPIO->BSRR=KEEP_LED_PIN;
		}
		
		led_animation_pwm();
		
		TIM14_PERIOD_GPIO->BRR=TIM14_PERIOD_PIN;
    }
}


