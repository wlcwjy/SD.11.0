#include "system.h"
#include "board.h"
#include "trace.h"
#include <string.h>
#include "crontab.h"
#include "flash.h"

#define KEY_MASK_DEF    			0x00
#define KEY_START_MASK     			0x01
#define KEY_KEEP_MASK      			0x02

#define KEY_TIMES_1_5S				1500
#define KEY_TIMES_5MS				5

crontab_variate_t crontab_variate;


//const static uint8_t FILE_NAME[]="crontab";


void crontab_config(void)
{
	

}

void crontab_update_key_value(crontab_variate_t *cv,key_value_e key_value)
{
	system_variate_t *sv=system_get_variate();
	
	cv->kettle_key_value=key_value;
	cv->timer_key_value=key_value;
	cv->temperature_unit_value=key_value;
	
	if((key_value==KEY_VAL_LP_START)||(key_value==KEY_VAL_START))
	{
		sv->run_info.start_key_count++;
	}
	if((key_value==KEY_VAL_KEEP)||(key_value==KEY_VAL_LP_KEEP))
	{
		sv->run_info.keep_key_count++;
	}
}

key_value_e crontab_timer_get_key_value(void)
{
	key_value_e key_value=crontab_variate.timer_key_value;
	
	crontab_variate.timer_key_value=KEY_VAL_NULL;
	
	return key_value;
}

key_value_e crontab_kettle_get_key_value(void)
{
	key_value_e key_value=crontab_variate.kettle_key_value;
	
	crontab_variate.kettle_key_value=KEY_VAL_NULL;
	
	return key_value;
}

key_value_e crontab_temperature_unit_get_key_value(void)
{
	key_value_e key_value=crontab_variate.temperature_unit_value;
	
	crontab_variate.temperature_unit_value=KEY_VAL_NULL;
	
	return key_value;
}

static void crontab_key_check_start(crontab_variate_t *cv)
{
	//static uint16_t key_count=0;
	static uint16_t push_count=0;
	static uint8_t last_status=0;
	system_variate_t *sv=system_get_variate();
   
	// if((KEEP_KEY_GPIO->IDR&KEEP_KEY_PIN)==0)
	// {
	// 	push_count=0;
	// 	key_count=0;
	// 	return;
	// }

	if(cv->start_key_count>KEY_TIMES_5MS)
	{
		
		push_count++;
		if((START_KEY_GPIO->IDR&START_KEY_PIN)==0)
		{
			if(push_count==KEY_TIMES_1_5S)
			{
				if(cv->keep_key_count)
				{
					sv->run_info.start_key_count++;
				}
				else
				{
					crontab_update_key_value(cv,KEY_VAL_LP_START);
				}
				
			}
			else if(push_count>KEY_TIMES_1_5S)
			{
				push_count=KEY_TIMES_1_5S;
			}
		}
		else
		{
			if(push_count<KEY_TIMES_1_5S)
			{
				if(cv->keep_key_count)
				{
					sv->run_info.start_key_count++;
				}
				else
				{
					crontab_update_key_value(cv,KEY_VAL_START);
				}
			}
			cv->start_key_count=0;
		}
		return;
	}

    push_count=0;
    if((START_KEY_GPIO->IDR&START_KEY_PIN)==0)
	{
		if(last_status==0)
		{
			cv->start_key_count++;
			
		}
		else
		{
			cv->start_key_count=0;
			last_status=0;
		}
    }
	else
	{
		cv->start_key_count=0;
	}
}

static void crontab_key_check_keep(crontab_variate_t *cv)
{
	//static uint16_t key_count=0;
	static uint16_t push_count=0;
	static uint8_t last_status=0;
	system_variate_t *sv=system_get_variate();
   
	// if((START_KEY_GPIO->IDR&START_KEY_PIN)==0)
	// {
	// 	push_count=0;
	// 	key_count=0;
	// 	return;
	// }

	if(cv->keep_key_count>KEY_TIMES_5MS)
	{
		push_count++;
		if((KEEP_KEY_GPIO->IDR&KEEP_KEY_PIN)==0)
		{
			if(push_count==KEY_TIMES_1_5S)
			{
				if(cv->start_key_count)
				{
					sv->run_info.keep_key_count++;
				}
				else
				{
					crontab_update_key_value(cv,KEY_VAL_LP_KEEP);
				}
			}
			else if(push_count>KEY_TIMES_1_5S)
			{
				push_count=KEY_TIMES_1_5S;
			}
		}
		else
		{
			if(push_count<KEY_TIMES_1_5S)
			{
				if(cv->start_key_count)
				{
					sv->run_info.keep_key_count++;
				}
				else
				{
					crontab_update_key_value(cv,KEY_VAL_KEEP);
				}
			}
			cv->keep_key_count=0;
		}
		return;
	}

	push_count=0;
    if((KEEP_KEY_GPIO->IDR&KEEP_KEY_PIN)==0)
	{
		if(last_status==0)
		{
			cv->keep_key_count++;
			
		}
		else
		{
			cv->keep_key_count=0;
			last_status=0;
		}
    }
	else
	{
		cv->keep_key_count=0;
	}
}

static void crontab_ac_lost_check(crontab_variate_t *cv)
{
	system_variate_t *sv=system_get_variate();
	static uint16_t second_delay=0;
	static uint32_t write_run_info_delay=0;
	static uint8_t first_in=0;
	
	if(sv->kettle_mode!=kettle_power_down)
	{
		write_run_info_delay=(CRONTAB_TIME_1H-CRONTAB_TIME_10S);
	}
	else
	{
		write_run_info_delay++;
		if(write_run_info_delay>=CRONTAB_TIME_1H)
		{
			if((cv->kettle_in_delay==0)&&(cv->kettle_remove_delay==0)
			&&(cv->key_pressed_delay==0))
			{
				write_run_info_delay=0;
				flash_write_run_info(&sv->run_info);
				sv->flash_run_time_s=sv->run_info.run_time_s;
			}
			else
			{
				write_run_info_delay=CRONTAB_TIME_1H;
			}
		}
	}
	
	if(sv->relay_error1_delay>0)
	{
		sv->relay_error1_delay--;
	}

	if(sv->relay_error2_delay>0)
	{
		sv->relay_error2_delay--;
	}
	
	second_delay++;
	if(second_delay>CRONTAB_TIME_1S)
	{
		second_delay=0;
		sv->run_info.run_time_s=sv->tick/100;
	}
	
	sv->ac_lost_tick++;
	if(sv->ac_lost_tick>CRONTAB_TIME_80MS)
	{
		sv->ac_lost_tick=CRONTAB_TIME_80MS;
		if(first_in==0)
		{
			flash_write_run_info(&sv->run_info);
			first_in=1;
		}
		else if((sv->run_info.run_time_s-sv->flash_run_time_s)>30)
		{
			flash_write_run_info(&sv->run_info);
			sv->flash_run_time_s=sv->run_info.run_time_s;
		}
	}
}

static void crontab_kettle_remove_check(crontab_variate_t *cv)
{
	system_variate_t *sv=system_get_variate();
	
	
	if(sv->busy_mark&KETTLE_BUSY)
	{
		cv->kettle_in_delay=0;
		if(sv->temperature<KETTLY_REMOVE_TEMPERATURE)
		{
			cv->kettle_remove_delay++;
			if(cv->kettle_remove_delay>CRONTAB_TIME_100MS)
			{
				sv->busy_mark&=~KETTLE_BUSY;
			}
		}
		else
		{
			cv->kettle_remove_delay=0;
		}
	}
	else
	{
		cv->kettle_remove_delay=0;
		if(sv->temperature>KETTLY_IN_TEMPERATURE)
		{
			cv->kettle_in_delay++;
			if(cv->kettle_in_delay>CRONTAB_TIME_500MS)
			{
				sv->busy_mark|=KETTLE_BUSY;
			}
		}
		else
		{
			cv->kettle_in_delay=0;
		}
	}
}

static void crontab_encoder_idle_check(crontab_variate_t *cv)
{
	system_variate_t *sv=system_get_variate();
	
	if(sv->busy_mark&ENCODER_BUSY)
	{
		if(sv->encoder_voice_delay>0)
		{
			sv->encoder_voice_delay--;
		}
		sv->encoder_idle_delay++;
		if(sv->encoder_idle_delay>CRONTAB_TIME_500MS)
		{
			sv->busy_mark&=~ENCODER_BUSY;
		}
	}
	else
	{
		sv->encoder_idle_delay=0;
		sv->encoder_voice_delay=0;
	}
}

// static void crontab_check_relay(crontab_variate_t *cv)
// {
// 	static uint8_t error1_count=0;
// 	static uint8_t error2_count=0;
// 	system_variate_t *sv=system_get_variate();
	
// 	if(sv->control_mode==control_boiling)
// 	{
// 		TEST_GPIO->ODR^=TEST_PIN;
// 		if(RELAY_FAIL_GPIO->IDR&RELAY_FAIL_PIN)
// 		{
// 			error1_count++;
// 			if(error1_count>CRONTAB_TIME_20MS)
// 			{
// 				error1_count=0;
// 				sv->relay_error1_count++;
// 				sv->relay_error1_delay=CRONTAB_TIME_20MS;
// 			}
// 		}
// 		else
// 		{
// 			sv->relay_error1_count=0;
// 			error1_count=0;
// 		}
// 	}
// 	else
// 	{
// 		//继电器没应该一直高电平
// 		TEST1_GPIO->ODR^=TEST1_PIN;
// 		if(RELAY_FAIL_GPIO->IDR&RELAY_FAIL_PIN)
// 		{
// 			sv->relay_error2_count=0;
// 			error2_count=0;
// 		}
// 		else
// 		{
// 			error2_count++;
// 			if(error2_count>=CRONTAB_TIME_20MS)
// 			{
// 				error2_count=0;
// 				sv->relay_error2_count++;
// 				sv->relay_error2_delay=CRONTAB_TIME_20MS;
// 			}
// 		}
// 	}
// }

void crontab_action(void)
{
	static uint8_t count=0;
	system_variate_t *sv=system_get_variate();
	
	sv->tick_1ms++;
	crontab_key_check_start(&crontab_variate);
	crontab_key_check_keep(&crontab_variate);
	//crontab_check_relay(&crontab_variate);
	count++;
	if(count>=10)
	{
		sv->tick++;
		count=0;
		crontab_ac_lost_check(&crontab_variate);
		crontab_kettle_remove_check(&crontab_variate);
		crontab_encoder_idle_check(&crontab_variate);
	}
}







