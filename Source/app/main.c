/* app_main.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com/wlcwjy@163.com
 * 日期：2024年3月8日
 * 提供工程入口函数以及初始化工程
 */
/* Files include */
#include "hal_conf.h"
#include "board.h"
#include "delay.h"
#include "aip33620.h"
#include "crontab.h"
#include "trace.h"
#include "flash.h"
#include "voice.h"
#include "led.h"
#include "led_animation.h"
/* Private typedef ****************************************************************************************************/

/* Private define *****************************************************************************************************/
//#define	TEST_RELAY
//#define	TEST_SELF
//#define 	TEST_FCT
/* Private macro ******************************************************************************************************/

/* Private variables **************************************************************************************************/
system_variate_t system_variate;
/* Private functions **************************************************************************************************/
#define MAIN_TIME_100MS					1
#define MAIN_TIME_200MS					2
#define MAIN_TIME_500MS					5
#define MAIN_TIME_800MS					8
#define MAIN_TIME_1S					10
#define MAIN_TIME_3S					30
#define MAIN_TIME_4S					40
#define MAIN_TIME_5S					50
#define MAIN_TIME_8S					80
#define MAIN_TIME_30S					300
#define MAIN_TIME_60S					600
#define MAIN_TIME_120S					1200
#define MAIN_TIME_3MIN					1800
#define MAIN_TIME_5MIN					3000
#define MAIN_TIME_15MIN					9000
#define MAIN_TIME_30MIN					18000

system_variate_t *system_get_variate(void)
{
	return &system_variate;
}

void system_reset_pid(system_variate_t *sv)
{
	if(sv->boil_pid.set_duty==0)
	{
		sv->boil_pid.kp=BOIL_PID_KP-(sv->set_temperature-50)*20;
		sv->boil_pid.ki=BOIL_PID_KI-(sv->set_temperature-50);
		sv->boil_pid.kd=BOIL_PID_KD;
		sv->boil_pid.current_error=0;
		sv->boil_pid.last_error=0;
		sv->boil_pid.previous_error=0;										
		sv->boil_pid.boil_pid_count=0;
		sv->boil_pid.current_duty=0;
		sv->boil_pid.current_duty=0;
		sv->boil_pid.first_in_flag=0;
	}
}

void system_boil_off(system_variate_t *sv)
{
	if(sv->boil_pid.set_duty==0)
	{
		sv->boil_mode=boil_off;
	}
}



//增量式PID
int32_t system_run_boil_pid(pid_t *pid, int32_t current_value, int32_t point)
{ 															//当前误差
	int32_t increase;														//量后得出的实际增量


	pid->current_error = point- current_value; 													// 计算当前误差

	increase =pid->kp * (pid->current_error- pid->last_error);							//比例
	increase+=pid->ki * pid->current_error;											//积分工
	increase+=pid->kd *(pid->current_error -(pid->last_error<<1) +pid->previous_error); 	//微分D

	pid->previous_error = pid->last_error; 									// 更新前次误差
	pid->last_error = pid->current_error;										//??更新上次误差

	increase>>=12;
	if(increase>BOIL_PID_MAX_STEP)
	{
		increase=BOIL_PID_MAX_STEP;
	}

	if(increase<(BOIL_PID_MAX_STEP*(-1)))
	{
		increase=(BOIL_PID_MAX_STEP*(-1));
	}
	return increase; 													//??返回增量
}

void system_run_pid_control(system_variate_t *sv)
{
	int32_t pwm_increase;
	uint8_t f=0;
	
	if(sv->boil_pid.set_duty!=0)
	{
		return;
	}

	if(sv->set_temperature<90)
	{
		if((sv->set_temperature-sv->show_temperature)>20)
		{
			f=1;	
		}
	}
	else
	{
		if((sv->set_temperature-sv->show_temperature)>15)
		{
			f=1;	
		}
	}

	if(f)
	{
		sv->boil_mode=boil_on;
		sv->boil_pid.current_duty=BOIL_PID_PREIOD;
	}
	else
	{
		if(sv->boil_pid.boil_pid_count==0)
		{

			pwm_increase=system_run_boil_pid(&sv->boil_pid,sv->temperature,sv->set_temperature*100);
			sv->boil_pid.current_duty+=pwm_increase;
			if(sv->boil_pid.current_duty>BOIL_PID_PREIOD)
			{
				sv->boil_pid.current_duty=BOIL_PID_PREIOD;
			}
			else if(sv->boil_pid.current_duty<MIN_BOIL_PID_PREIOD)
			{
				sv->boil_pid.current_duty=MIN_BOIL_PID_PREIOD;
			}
			if(sv->boil_pid.first_in_flag)
			{
				if(sv->temperature>((sv->set_temperature*100)-KETTLY_TCOMP))
				{
					if(sv->boil_pid.current_duty>MIN_BOIL_PID_PREIOD)
					{
						sv->boil_pid.current_duty=MIN_BOIL_PID_PREIOD;
						sv->boil_pid.boil_pid_count=MIN_BOIL_PID_PREIOD;
					}
				}
			}
		}
		if(sv->boil_pid.boil_pid_count>sv->boil_pid.current_duty)
		{
			sv->boil_mode=boil_pause;
		}
		else
		{
			sv->boil_mode=boil_on;
		}
	}

	sv->boil_pid.boil_pid_count++;
	if(sv->boil_pid.boil_pid_count>=BOIL_PID_PREIOD)
	{
		sv->boil_pid.boil_pid_count=0;
		sv->boil_pid.first_in_flag=1;
	}
}

#ifdef TEST_RELAY

static void system_check_relay(system_variate_t *sv)
{
	static uint32_t tick=0;
	static uint32_t run_count=0;
	
	if((sv->tick-tick)>CRONTAB_TIME_6S)
	{
		tick=sv->tick;
		run_count++;
		if(run_count>9999)
		{
			run_count=0;
		}
		sv->boil_mode=boil_pause;
		display_update_number(run_count);
	}
	else if((sv->tick-tick)==CRONTAB_TIME_1S)
	{
		sv->boil_mode=boil_on;
	}
}

#else
static void run_kettle_power_down(system_variate_t *sv)
{
	static uint8_t power_down_flag=0;
	static int16_t power_off_busy_mark=0;
	static uint8_t wait_time=0;
	int16_t t=0;
	key_value_e key_value;

	if((sv->busy_mark&POWER_DOWN_BUSY))
	{
		if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
		{
			sv->kettle_tick=sv->tick;
			wait_time++;
			if(wait_time>=MAIN_TIME_4S)
			{
				key_value=crontab_kettle_get_key_value();
				sv->busy_mark&=~POWER_DOWN_BUSY;
			}
			if(animation_2==led_get_animation(animation_lighting))
			{
				led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
			}
			if(sv->kettle_last_mode==kettle_before_keep_warm)
			{
				if(animation_2==led_get_animation(animation_nixie_tube))
				{
					led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
					led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
				}
			}
			
			if(sv->kettle_last_mode==kettle_keep_warm_boiling_water)
			{
				if((animation_2==led_get_animation(animation_keep_led)&&(animation_2==led_get_animation(animation_nixie_tube))))
				{
					led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
					led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
				}
			}

			if((animation_1==led_get_animation(animation_lighting))&&(animation_1==led_get_animation(animation_nixie_tube))&&
			(animation_1==led_get_animation(animation_music_led))&&(animation_1==led_get_animation(animation_brightness_led))&&
			(animation_1==led_get_animation(animation_keep_led)))
			{
				key_value=crontab_kettle_get_key_value();
				sv->kettle_tick=sv->tick;
				sv->busy_mark&=~POWER_DOWN_BUSY;
			}
		}
		return;
	}
	else
	{
		wait_time=0;
	}

	if((sv->tick-sv->kettle_tick)>CRONTAB_TIME_100MS)
	{
		sv->kettle_tick=sv->tick;
		ENCODER_OFF();
//		display_nixie_tube_off();
		system_reset_pid(sv);
		led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
		led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
		led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
		led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
		led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
		system_boil_off(sv);
		if(power_down_flag==0)
		{
			power_down_flag=1;
			voice_set_voice(voice1);
			power_off_busy_mark=sv->busy_mark;
		}

		//单位转换
		if(sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)
		{
			sv->set_fahrenheit_temperature=sv->set_info.temperature;
			t=(sv->set_info.temperature-32)*50/9;
			sv->set_temperature=t/10;
			if((t%10)>=5)
			{
				sv->set_temperature++;
			}
		}
		else
		{
			sv->set_fahrenheit_temperature=sv->set_info.temperature*9/5+32;
			if(sv->set_fahrenheit_temperature&0x01)
			{
				sv->set_fahrenheit_temperature++;
			}
			sv->set_temperature=sv->set_info.temperature;
		}
	}
	
	key_value=crontab_kettle_get_key_value();
	//The kettle remove check
	if(power_down_flag)
	{
		if((power_off_busy_mark&KETTLE_BUSY)!=(sv->busy_mark&KETTLE_BUSY))
		{
			if(sv->busy_mark&KETTLE_BUSY)
			{
				power_off_busy_mark=sv->busy_mark;
			}
			else
			{
				voice_set_voice(voice7);
				led_set_animation(animation_lighting,animation_6_to_1,sv->set_info.ring_duty);
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_before_boiling;
				sv->kettle_tick=sv->tick;
				power_down_flag=0;
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				voice_play(sv->set_info.music_type,voice1);
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_40MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				ENCODER_ON();
			}
		}
	}
	switch(key_value)
	{
		case KEY_VAL_START:
		case KEY_VAL_KEEP:
			voice_set_voice(voice7);
			led_set_animation(animation_lighting,animation_6_to_1,sv->set_info.ring_duty);
			sv->encoder_priority=encoder_temperature;
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_before_boiling;
			sv->kettle_tick=sv->tick;
			power_down_flag=0;
			sv->kettle_tick=sv->tick;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice1);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_40MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			ENCODER_ON();
			break;
		case KEY_VAL_LP_KEEP:
			voice_set_voice(voice7);
			led_set_animation(animation_music_led,animation_3_to_2,sv->set_info.indicator_duty);
			sv->encoder_priority=encoder_music_type;
			sv->set_music_type=sv->set_info.music_type;
			ENCODER_ON();
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_setting_music_type;
			sv->kettle_tick=sv->tick;
			power_down_flag=0;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice1);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_40MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			ENCODER_ON();
			break;
		default:break;
	}
}

static void run_kettle_before_boiling(system_variate_t *sv)
{
	static uint16_t power_off_delay=0;
	key_value_e key_value;

	//100MS的计时周期时间到了
	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		sv->kettle_tick=sv->tick;
		system_reset_pid(sv);
		system_boil_off(sv);
		if(sv->busy_mark&ENCODER_BUSY)
		{
			//拨码开关工作中
			power_off_delay=0;
			display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
		}
		else
		{
			//等待设置
			display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	
			power_off_delay++;
			if(power_off_delay>MAIN_TIME_60S)
			{
				if(sv->kettle_mode==kettle_before_keep_warm)
				{
					if(animation_2==led_get_animation(animation_nixie_tube))
					{
						led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
						led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
						sv->kettle_last_mode=sv->kettle_mode;
						sv->kettle_mode=kettle_power_down;
						sv->busy_mark|=POWER_DOWN_BUSY;
						power_off_delay=0;
					}
					else
					{
						led_set_animation(animation_nixie_tube,animation_6_to_2,sv->set_info.seg_duty);
					}	
				}
				else
				{
					led_set_animation(animation_nixie_tube,animation_6_to_1,sv->set_info.indicator_duty);
					sv->kettle_last_mode=sv->kettle_mode;
					sv->kettle_mode=kettle_power_down;
					sv->busy_mark|=POWER_DOWN_BUSY;
					power_off_delay=0;
				}
				
			}
			else
			{
				if((animation_1==led_get_animation(animation_nixie_tube))||(animation_2==led_get_animation(animation_nixie_tube)))
				{
					led_set_animation(animation_nixie_tube,animation_6,sv->set_info.seg_duty);
				}
			}
		}
		if(animation_2==led_get_animation(animation_lighting))
		{
			led_set_animation(animation_lighting,animation_4_to_1,sv->set_info.ring_duty);
		}
		

		//判断是否继电器错误
		if(sv->relay_error1_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_001_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->error_mark|=ERROR_E02_001;
			trace_printf("E02(001) Relay Welding_Unintentional Activate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
		}
	}

	key_value=crontab_kettle_get_key_value();
	switch(key_value)
	{
		case KEY_VAL_START:
			power_off_delay=0;
			if(sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)
			{
				if(sv->set_fahrenheit_temperature!=sv->set_info.temperature)
				{
					sv->set_info.temperature=sv->set_fahrenheit_temperature;
					flash_write_set_info(&sv->set_info);
				}
			}
			else
			{
				if(sv->set_temperature!=sv->set_info.temperature)
				{
					sv->set_info.temperature=sv->set_temperature;
					flash_write_set_info(&sv->set_info);
				}
			}
			//设置的温度大于当前水温,报错U03
			if(sv->show_temperature>=sv->set_temperature)
			{
				sv->run_info.u03_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11_to_1,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_6_to_1,sv->set_info.ring_duty);
				if(sv->kettle_mode==kettle_before_keep_warm)
				{
					led_set_animation(animation_keep_led,animation_4_to_1,sv->set_info.indicator_duty);
				}
				led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
				led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
				display_update_current_temperature(sv->show_temperature,sv->set_info.temperature_unit);
				sv->error_mark|=ERROR_U03;
				trace_printf("U03 Preset Temperature Error!Temperature:%d\r\n",sv->temperature);
				ENCODER_OFF();
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				power_off_delay=0;
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
			//无壶干烧,报错U01
			else if((sv->busy_mark&KETTLE_BUSY)==0)
			{
				sv->run_info.u01_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
				led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
				led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
				led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
				sv->error_mark|=ERROR_U01;
				ENCODER_OFF();
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				trace_printf("U01 Not Detected!\r\n");
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
			else
			{
				//开始煮水了
				led_set_animation(animation_nixie_tube,animation_6_to_1,sv->set_info.seg_duty);
				sv->kettle_last_mode=sv->kettle_mode;
				if(sv->kettle_mode==kettle_before_keep_warm)
				{
					sv->kettle_mode=kettle_keep_warm_boiling_water;
					led_set_animation(animation_keep_led,animation_4_to_1,sv->set_info.indicator_duty);
				}
				else
				{
					sv->kettle_mode=kettle_boiling_water;
				}
				system_reset_pid(sv);
				sv->boil_pid.current_duty=(sv->set_temperature-sv->show_temperature)*40;
				if(sv->boil_pid.current_duty>BOIL_PID_PREIOD)
				{
					sv->boil_pid.current_duty=BOIL_PID_PREIOD;
				}
				ENCODER_OFF();
				sv->u02_error_time_count=0;
				sv->e03_error_time_count=0;
				sv->e04_error_time_count=0;
				sv->plateau_boiling_count=0;
				sv->max_boiling_temperature=sv->show_temperature;
				sv->initial_temperature=sv->temperature;
				sv->last_show_temperature=sv->show_temperature;
				sv->busy_mark&=~BOILED_BUSY;
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick));
				voice_play(sv->set_info.music_type,voice2);
				sv->kettle_tick=sv->tick;
			}
			break;
		case KEY_VAL_KEEP:
			//进入保温模式
			power_off_delay=0;
			if(sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)
			{
				if(sv->set_fahrenheit_temperature!=sv->set_info.temperature)
				{
					sv->set_info.temperature=sv->set_fahrenheit_temperature;
					flash_write_set_info(&sv->set_info);
				}
			}
			else
			{
				if(sv->set_temperature!=sv->set_info.temperature)
				{
					sv->set_info.temperature=sv->set_temperature;
					flash_write_set_info(&sv->set_info);
				}
			}
			sv->kettle_last_mode=sv->kettle_mode;
			led_set_animation(animation_nixie_tube,animation_6,sv->set_info.seg_duty);
			if(sv->kettle_mode==kettle_before_keep_warm)
			{
				led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
				sv->kettle_mode=kettle_before_boiling;
			}
			else
			{
				led_set_animation(animation_keep_led,animation_3_to_2,sv->set_info.indicator_duty);
				sv->kettle_mode=kettle_before_keep_warm;
			}	
			system_reset_pid(sv);
			display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice1);
			break;
		case KEY_VAL_LP_KEEP:
			power_off_delay=0;
			//进入设置模式
			voice_play(sv->set_info.music_type,voice2);
			led_set_animation(animation_music_led,animation_3_to_2,sv->set_info.indicator_duty);
			if(sv->kettle_mode==kettle_before_keep_warm)
			{
				led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
			}
			sv->encoder_priority=encoder_music_type;
			sv->set_music_type=sv->set_info.music_type;
			ENCODER_ON();
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_setting_music_type;
			sv->kettle_tick=sv->tick;
			break;
		default:break;
	}
}

static void run_kettle_boiling_water(system_variate_t *sv)
{
	static uint8_t boiled_delay=0;
	static uint8_t voice_delay=0;
	static uint8_t temperature_update_delay=0;
	static uint8_t u02_error_delay=0;
	static uint8_t e01_error_delay=0;
	static uint32_t pid_tick=0;
	static uint8_t e03_error_delay=0;
	key_value_e key_value;
	uint8_t t=0;
	uint8_t boiled_delay_time=0;

	if(sv->busy_mark&KEEP_WARN_BOILING_STOP_BUSY)
	{
		if(animation_2==led_get_animation(animation_keep_led))
		{
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
		}
		if(animation_1==led_get_animation(animation_keep_led))		
		{
			sv->kettle_mode=kettle_before_boiling;
			sv->kettle_last_mode=sv->kettle_mode;
			display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	
			sv->busy_mark&=~KEEP_WARN_BOILING_STOP_BUSY;
		}
		return;
	}

	if(sv->busy_mark&BOILED_BUSY)
	{
		if(sv->kettle_mode==kettle_keep_warm_boiling_water)
		{
			if((animation_1==led_get_animation(animation_keep_led))&&(animation_1==led_get_animation(animation_nixie_tube)))
			{
				led_set_animation(animation_keep_led,animation_3_to_2,sv->set_info.indicator_duty);
				led_set_animation(animation_nixie_tube,animation_3_to_2,sv->set_info.seg_duty);
			}
		}
		else
		{
			if(animation_1==led_get_animation(animation_nixie_tube))
			{
				led_set_animation(animation_nixie_tube,animation_3_to_2,sv->set_info.seg_duty);
			}
		}
		
		if(animation_2==led_get_animation(animation_lighting))
		{
			led_set_animation(animation_lighting,animation_9_to_5,sv->set_info.ring_duty);
		}
		if(animation_1==led_get_animation(animation_lighting))
		{
			sv->kettle_last_mode=sv->kettle_mode;
			if(sv->kettle_mode==kettle_keep_warm_boiling_water)
			{
				sv->kettle_mode=kettle_keep_warm_boiled_water;
			}
			else
			{
				sv->kettle_mode=kettle_boiled_water;
			}
			sv->busy_mark&=~BOILED_BUSY;
			sv->keep_warn_change_delay=0;
			led_set_animation(animation_lighting,animation_13,sv->set_info.ring_duty);
		}
		system_reset_pid(sv);
		return;
	}
	
	//100MS的计时周期时间到了
	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		boiled_delay++;
		sv->kettle_tick=sv->tick;
		if(animation_6_to_1!=led_get_animation(animation_nixie_tube))
		{
			temperature_update_delay++;
			if(temperature_update_delay==MAIN_TIME_800MS)
			{
				temperature_update_delay=0;
				if(((sv->set_temperature-sv->last_show_temperature)>=5)&&((sv->set_temperature-sv->show_temperature)>=5))
				{
					t=sv->last_show_temperature;
					t+=5;
					t/=5;
					t*=5;
					if(sv->show_temperature>=t)
					{
						sv->last_show_temperature=t;
						display_update_current_temperature(sv->last_show_temperature,sv->set_info.temperature_unit);
					}
				}
				else
				{
					if((sv->last_show_temperature<90)&&((sv->set_temperature-sv->last_show_temperature)>=10))
					{
						t=sv->last_show_temperature;
						t+=5;
						t/=5;
						t*=5;
						if(sv->show_temperature>=t)
						{
							sv->last_show_temperature=t;
							display_update_current_temperature(sv->last_show_temperature,sv->set_info.temperature_unit);
						}
					}
					else
					{
						if(sv->show_temperature>sv->last_show_temperature)
						{
							sv->last_show_temperature+=1;
							if(sv->last_show_temperature>sv->set_temperature)
							{
								sv->last_show_temperature=sv->set_temperature;
							}
							display_update_current_temperature(sv->last_show_temperature,sv->set_info.temperature_unit);
						}
					}
				}
				
			}
		}
		
		if(animation_1==led_get_animation(animation_nixie_tube))
		{
			display_update_current_temperature(sv->last_show_temperature,sv->set_info.temperature_unit);
			led_set_animation(animation_nixie_tube,animation_3_to_2,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_3_to_7,sv->set_info.ring_duty);
		}

		if(sv->kettle_mode==kettle_keep_warm_boiling_water)
		{
			led_set_animation(animation_keep_led,animation_6,sv->set_info.indicator_duty);
		}

		//判断下是否干烧其中一个原因
		if(sv->temperature>=MAX_TEMPERATURE)
		{
			if(u02_error_delay>=MAIN_TIME_500MS)
			{
				sv->run_info.u02_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
				sv->error_mark|=ERROR_U02;
				sv->error_mark|=ERROR_U02_001;
				trace_printf("U02 Heat Without Water!AD:%d\r\n",sv->temperature_adc);
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				system_boil_off(sv);
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
		}
		else
		{
			u02_error_delay=0;;
		}
		//判断下是否干烧另外一个原因
		if(sv->u02_error_time_count<MAIN_TIME_30S)
		{
			sv->u02_error_time_count++;
			if((sv->temperature-sv->initial_temperature)>DRY_FIRING_TEMPERATURE)
			{
				sv->run_info.u02_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
				sv->error_mark|=ERROR_U02;
				sv->error_mark|=ERROR_U02_002;
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				system_boil_off(sv);
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				trace_printf("U02 Heat Without Water!AD:%d\r\n",sv->temperature_adc);
				voice_play(sv->set_info.music_type,voice7);
				sv->u02_error_time_count=0;
				sv->kettle_tick=sv->tick;
			}
		}
		else
		{
			sv->u02_error_time_count=MAIN_TIME_30S;
		}

		//判断NTC故障
		if(sv->temperature>=NTC_ERROR_TEMPERATURE)
		{
			e01_error_delay++;
			if(e01_error_delay>=MAIN_TIME_1S)
			{	
				sv->run_info.e01_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_11,sv->set_info.ring_duty);
				sv->error_mark|=ERROR_E01;
				trace_printf("E01 Thermistor Failure!AD:%d\r\n",sv->temperature_adc);
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				system_boil_off(sv);
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
		}
		else
		{
			e01_error_delay=0;;
		}
		//判断发热芯是否故障了
		if(sv->e03_error_time_count<MAIN_TIME_3MIN)
		{
			sv->e03_error_time_count++;
		}
		else
		{
			if(sv->busy_mark&KETTLE_BUSY)
			{
				if((sv->temperature-sv->initial_temperature)<HEATING_AGING_TEMPERATURE)
				{
					e03_error_delay++;
					if(e03_error_delay>MAIN_TIME_1S)
					{
						sv->run_info.e03_count++;
						flash_write_run_info(&sv->run_info);
						led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
						led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
						trace_printf("E03 Heater Failure! AD:%d\r\n",sv->temperature);
						sv->error_mark|=ERROR_E03;
						sv->kettle_last_mode=sv->kettle_mode;
						sv->kettle_mode=kettle_error;
						system_boil_off(sv);
						sv->kettle_tick=sv->tick;
						while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
						{
							led_run_animation();
						}
						voice_play(sv->set_info.music_type,voice7);
						sv->kettle_tick=sv->tick;
					}
				}
			}
		}

		//判断发热芯是否老化
		if(sv->set_temperature<90)
		{
			if(sv->e04_error_time_count<MAIN_TIME_15MIN)
			{
				sv->e04_error_time_count++;
			}
			else
			{
				sv->run_info.e04_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
				sv->error_mark|=ERROR_E04;
				trace_printf("E04 Heater deterioration! AD:%d\r\n",sv->temperature);
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				system_boil_off(sv);
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
		}

		//判断高原煮水
		if(sv->max_boiling_temperature>90)
		{
			if(sv->show_temperature>sv->max_boiling_temperature)
			{	
				sv->max_boiling_temperature=sv->show_temperature;
				sv->plateau_boiling_count=0;
			}
			else
			{
				sv->plateau_boiling_count++;
				if(sv->plateau_boiling_count>MAIN_TIME_60S)
				{
					sv->run_info.plateau_boiling_count++;
					led_set_animation(animation_nixie_tube,animation_11_to_1,sv->set_info.seg_duty);
					led_set_animation(animation_lighting,animation_8_to_2,sv->set_info.ring_duty);
					if(sv->kettle_mode==kettle_keep_warm_boiling_water)
					{
						led_set_animation(animation_keep_led,animation_6_to_1,sv->set_info.indicator_duty);
					}
					boiled_delay=0;
					sv->run_info.boiled_count++;
					system_boil_off(sv);
					sv->kettle_tick=sv->tick;
					sv->busy_mark|=BOILED_BUSY;
				}
			}
		}
		else
		{
			sv->plateau_boiling_count=0;
			if(sv->show_temperature>sv->max_boiling_temperature)
			{	
				sv->max_boiling_temperature=sv->show_temperature;
			}
		}

		//判断是否继电器错误
		if(sv->relay_error2_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_001_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->run_info.e02_001_count++;
			sv->relay_error2_count=0;
			sv->error_mark|=ERROR_E02_001;
			trace_printf("E02(001) Relay Welding_Unintentional Activate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
		}
		if(sv->relay_error1_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_002_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->error_mark|=ERROR_E02_002;
			sv->relay_error1_count=0;
			trace_printf("E02(002) Relay Welding_Inactivate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
		}

		voice_delay++;
		if(voice_delay>=MAIN_TIME_1S)
		{
			voice_delay=0;
			voice_play(sv->set_info.music_type,voice5);
		}
	}	
	
	//每10MS运行一次PID程序
	if(pid_tick!=sv->tick)
	{
		pid_tick=sv->tick;
		system_run_pid_control(sv);
	}

	//水煮好了
	if((sv->last_show_temperature>=(sv->set_temperature-1))||((sv->set_temperature==100)&&(sv->last_show_temperature>=99)))
	{
		if(sv->set_temperature==100)
		{
			boiled_delay_time=MAIN_TIME_8S;
		}
		else
		{
			if((sv->show_temperature-1)>=sv->set_temperature)
			{
				boiled_delay_time=MAIN_TIME_200MS;
			}
			else
			{
				boiled_delay_time=MAIN_TIME_8S;
			}
		}
		if(boiled_delay>=boiled_delay_time)
		{
			led_set_animation(animation_nixie_tube,animation_4_to_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_8_to_2,sv->set_info.ring_duty);
			if(sv->kettle_mode==kettle_keep_warm_boiling_water)
			{
				led_set_animation(animation_keep_led,animation_6_to_1,sv->set_info.indicator_duty);
			}
			boiled_delay=0;
			sv->max_boiling_temperature=0;
			sv->run_info.boiled_count++;
			system_boil_off(sv);
			system_reset_pid(sv);
			sv->kettle_tick=sv->tick;
			sv->busy_mark|=BOILED_BUSY;
		}
	}
	else
	{
		boiled_delay=0;
	}

	key_value=crontab_kettle_get_key_value();
	if(voice_delay==0)
	{
		return;
	}
	if((sv->busy_mark&KETTLE_BUSY)==0)
	{
		voice_play(sv->set_info.music_type,voice3);
		led_set_animation(animation_lighting,animation_8_to_2,sv->set_info.ring_duty);
		if(sv->kettle_mode==kettle_keep_warm_boiling_water)
		{
			if(animation_2!=led_get_animation(animation_nixie_tube))
			{
				led_set_animation(animation_nixie_tube,animation_3_to_2,sv->set_info.seg_duty);
			}
			led_set_animation(animation_keep_led,animation_6_to_1,sv->set_info.indicator_duty);
		}
		else
		{
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
		}
		system_boil_off(sv);
		sv->run_info.remove_count++;
		sv->kettle_last_mode=sv->kettle_mode;
		sv->kettle_mode=kettle_power_down;
		sv->busy_mark|=POWER_DOWN_BUSY;
		sv->kettle_tick=sv->tick;
		return;
	}

	switch(key_value)
	{
		case KEY_VAL_START:
			led_set_animation(animation_lighting,animation_8_to_2,sv->set_info.ring_duty);
			if(sv->kettle_mode==kettle_keep_warm_boiling_water)
			{
				led_set_animation(animation_keep_led,animation_6_to_2,sv->set_info.indicator_duty);	
			}
			else
			{
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			}
			voice_play(sv->set_info.music_type,voice3);
			system_boil_off(sv);
			sv->run_info.remove_count++;
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			break;
		case KEY_VAL_KEEP:
			voice_play(sv->set_info.music_type,voice2);
			sv->kettle_last_mode=sv->kettle_mode;
			system_reset_pid(sv);
			ENCODER_ON();
			system_boil_off(sv);
			led_set_animation(animation_lighting,animation_8_to_2,sv->set_info.ring_duty);
			if(sv->kettle_mode==kettle_keep_warm_boiling_water)
			{
				led_set_animation(animation_keep_led,animation_6_to_2,sv->set_info.indicator_duty);
				sv->busy_mark|=KEEP_WARN_BOILING_STOP_BUSY;
				return;
			}
			else
			{
				led_set_animation(animation_nixie_tube,animation_6,sv->set_info.seg_duty);
				led_set_animation(animation_keep_led,animation_3_to_2,sv->set_info.indicator_duty);
				sv->kettle_mode=kettle_before_keep_warm;
			}
			display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	
			break;
		default:break;
	}
}

static void run_kettle_boiled_water(system_variate_t *sv)
{
	static uint16_t wait_timer_s=0;
	static uint8_t kettle_remove_delay=0;
	static uint8_t kettle_remove_flag=0;
	key_value_e key_value;
	
	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		wait_timer_s++;
		kettle_remove_delay++;
		sv->keep_warn_change_delay++;
		sv->kettle_tick=sv->tick;
		system_boil_off(sv);
		if(kettle_remove_flag==0)
		{
			if(wait_timer_s>MAIN_TIME_5MIN)
			{
				sv->kettle_last_mode=sv->kettle_mode;
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
				sv->kettle_mode=kettle_power_down;
				sv->busy_mark|=POWER_DOWN_BUSY;	
				sv->kettle_tick=sv->tick;
				wait_timer_s=0;
			}
			if(sv->max_boiling_temperature!=0)
			{
				display_update_current_temperature(sv->max_boiling_temperature,sv->set_info.temperature_unit);
			}
			else
			{
				display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);
			}
		}
	}	
	if(kettle_keep_warm_boiled_water==sv->kettle_mode)
	{
		if(((sv->busy_mark&KETTLE_BUSY)==0)&&(kettle_remove_flag))
		{
			sv->keep_warn_change_delay=0;
		}
		if(sv->keep_warn_change_delay>MAIN_TIME_3S)
		{
			voice_play(sv->set_info.music_type,voice1);
			led_set_animation(animation_lighting,animation_4_to_10,sv->set_info.ring_duty*7/10);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_keep_warm;
			sv->run_info.keep_count++;
			sv->kettle_tick=sv->tick;
			system_reset_pid(sv);
			wait_timer_s=0;
			sv->keep_warm_wait_timer_s=0;
			return;
		}
	}

	key_value=crontab_kettle_get_key_value();
	if(kettle_remove_flag)
	{
		if(kettle_remove_delay>MAIN_TIME_3S)
		{
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			kettle_remove_delay=0;
			kettle_remove_flag=0;
		}
		return;
	}
	else if((sv->busy_mark&KETTLE_BUSY)==0)
	{
		voice_play(sv->set_info.music_type,voice3);
		if(animation_13==led_get_animation(animation_lighting))
		{
			led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
		}
		else if((animation_9_to_5!=led_get_animation(animation_lighting))&&(animation_1!=led_get_animation(animation_lighting)))
		{
			led_set_animation(animation_lighting,animation_9_to_5,sv->set_info.ring_duty);
		}

		if(kettle_keep_warm_boiled_water==sv->kettle_mode)
		{
			led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
		}
		led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
		system_boil_off(sv);
		sv->run_info.remove_count++;
		sv->kettle_last_mode=sv->kettle_mode;
		kettle_remove_flag=1;
		kettle_remove_delay=0;
		return;
	}
	else
	{
		kettle_remove_flag=0;
		kettle_remove_delay=0;
	}

	switch(key_value)
	{
		case KEY_VAL_START:
			voice_play(sv->set_info.music_type,voice3);
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			if(animation_13==led_get_animation(animation_lighting))
			{
				led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
			}
			else if((animation_9_to_5!=led_get_animation(animation_lighting))&&(animation_1!=led_get_animation(animation_lighting)))
			{
				led_set_animation(animation_lighting,animation_9_to_5,sv->set_info.ring_duty);
			}
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;	
			sv->kettle_tick=sv->tick;
			wait_timer_s=0;
			break;
		case KEY_VAL_KEEP:
			if(sv->kettle_last_mode==kettle_keep_warm_boiling_water)
			{
				break;
			}
			voice_play(sv->set_info.music_type,voice1);
			sv->kettle_last_mode=sv->kettle_mode;
			led_set_animation(animation_keep_led,animation_3_to_2,sv->set_info.indicator_duty);
			if(animation_13==led_get_animation(animation_lighting))
			{
				led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
			}
			else if(animation_8_to_2==led_get_animation(animation_lighting))
			{
				led_set_animation(animation_lighting,animation_9_to_5,sv->set_info.ring_duty);
			}
			sv->kettle_mode=kettle_keep_warm;
			sv->run_info.keep_count++;
			sv->kettle_tick=sv->tick;
			system_reset_pid(sv);
			wait_timer_s=0;
			sv->keep_warm_wait_timer_s=0;
			break;
		default:break;
	}
}

static void run_kettle_keep_warm(system_variate_t *sv)
{
	static uint32_t pid_tick=0;
	static uint16_t u02_error_delay=0;
	static uint16_t keep_warm_delay=0;
	key_value_e key_value;

	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		sv->kettle_tick=sv->tick;
		sv->keep_warm_wait_timer_s++;
		keep_warm_delay++;
		if(sv->keep_warm_wait_timer_s>MAIN_TIME_30MIN)
		{
			led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_10_to_1,sv->set_info.ring_duty);
			//voice_play(sv->set_info.music_type,voice3);
			system_boil_off(sv);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_keep_warm_time_out;
			sv->kettle_tick=sv->tick;
		}
		if((animation_10!=led_get_animation(animation_lighting))&&((animation_1==led_get_animation(animation_lighting))))
		{
			led_set_animation(animation_lighting,animation_10,sv->set_info.ring_duty*7/10);
		}
		//led_set_animation(animation_keep_led,animation_2,sv->set_info.indicator_duty);
		display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	

		//判断是否继电器错误
		if(sv->relay_error2_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_001_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->run_info.e02_001_count++;
			sv->relay_error2_count=0;
			sv->error_mark|=ERROR_E02_001;
			trace_printf("E02(001) Relay Welding_Unintentional Activate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
			sv->u02_error_time_count=0;
		}
		if(sv->relay_error1_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_002_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->run_info.e02_001_count++;
			sv->relay_error1_count=0;
			sv->error_mark|=ERROR_E02_002;
			trace_printf("E02(002) Relay Welding_Inactivate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
			sv->u02_error_time_count=0;
		}

		//判断下是否干烧其中一个原因
		if(sv->temperature>=MAX_TEMPERATURE)
		{
			if(u02_error_delay>=MAIN_TIME_500MS)
			{
				sv->run_info.u02_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
				sv->run_info.u02_count++;
				sv->error_mark|=ERROR_U02;
				sv->error_mark|=ERROR_U02_001;
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				system_boil_off(sv);
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				trace_printf("U02 Heat Without Water!AD:%d\r\n",sv->temperature_adc);
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
		}
		else
		{
			u02_error_delay=0;;
		}
		//判断下是否干烧另外一个原因
		if(sv->u02_error_time_count<MAIN_TIME_30S)
		{
			sv->u02_error_time_count++;
			if((sv->temperature-sv->initial_temperature)>DRY_FIRING_TEMPERATURE)
			{
				sv->run_info.u02_count++;
				flash_write_run_info(&sv->run_info);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
				sv->error_mark|=ERROR_U02;
				trace_printf("U02 Heat Without Water!AD:%d\r\n",sv->temperature_adc);
				sv->error_mark|=ERROR_U02_002;
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_error;
				system_boil_off(sv);
				sv->kettle_tick=sv->tick;
				sv->u02_error_time_count=0;
				sv->kettle_tick=sv->tick;
				while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
				{
					led_run_animation();
				}
				voice_play(sv->set_info.music_type,voice7);
				sv->kettle_tick=sv->tick;
			}
		}
		else
		{
			sv->u02_error_time_count=MAIN_TIME_30S;
		}

		//判断是否继电器错误
		if(sv->relay_error2_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_001_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->run_info.e02_001_count++;
			sv->relay_error2_count=0;
			sv->error_mark|=ERROR_E02_001;
			trace_printf("E02(001) Relay Welding_Unintentional Activate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
		}
		if(sv->relay_error1_count>MAX_RELAY_ERROR_COUNT)
		{
			sv->run_info.e02_002_count++;
			flash_write_run_info(&sv->run_info);
			led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_6,sv->set_info.ring_duty);
			sv->run_info.e02_001_count++;
			sv->relay_error1_count=0;
			sv->error_mark|=ERROR_E02_002;
			trace_printf("E02(002) Relay Welding_Inactivate Error! AD:%d\r\n",sv->temperature);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_error;
			system_boil_off(sv);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice7);
			sv->kettle_tick=sv->tick;
		}
	}
	
	if(sv->busy_mark&KEEP_WARM_BUSY)
	{
		if(sv->show_temperature>=sv->set_temperature)
		{
			sv->busy_mark&=~KEEP_WARM_BUSY;
			keep_warm_delay=0;
		}
		else if(pid_tick!=sv->tick)
		{
			pid_tick=sv->tick;
			system_run_pid_control(sv);
		}
	}
	else
	{
		pid_tick=sv->tick;
		system_boil_off(sv);
		system_reset_pid(sv);
		if((sv->set_temperature-sv->show_temperature)>2)
		{
			if(keep_warm_delay>MAIN_TIME_1S)
			{
				sv->busy_mark|=KEEP_WARM_BUSY;
				keep_warm_delay=0;
			}
		}
		else
		{
			keep_warm_delay=0;
		}
	}

	
	if((sv->busy_mark&KETTLE_BUSY)==0)
	{
		led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
		led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
		led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
		voice_play(sv->set_info.music_type,voice3);
		system_boil_off(sv);
		sv->run_info.remove_count++;
		sv->kettle_last_mode=sv->kettle_mode;
		sv->kettle_mode=kettle_power_down;
		sv->busy_mark|=POWER_DOWN_BUSY;
		sv->kettle_tick=sv->tick;
		return;
	}
		
	key_value=crontab_kettle_get_key_value();
	switch(key_value)
	{
		case KEY_VAL_KEEP:
		case KEY_VAL_START:
			led_set_animation(animation_keep_led,animation_5_to_1,sv->set_info.indicator_duty);
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
			voice_play(sv->set_info.music_type,voice3);
			system_boil_off(sv);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			break;
		default:break;
	}
}

static void run_kettle_keep_warm_time_out(system_variate_t *sv)
{
	if((animation_1==led_get_animation(animation_keep_led))&&(animation_1==led_get_animation(animation_nixie_tube))
	&&(animation_1==led_get_animation(animation_lighting)))
	{
		sv->kettle_last_mode=sv->kettle_mode;
		sv->kettle_mode=kettle_power_down;
	}
}

static void run_kettle_setting_music_type(system_variate_t *sv)
{
	static uint16_t wait_timer_s=0;
	key_value_e key_value;
	
	if(wait_timer_s>MAIN_TIME_60S)
	{
		if(animation_2==led_get_animation(animation_nixie_tube))
		{
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			led_set_animation(animation_music_led,animation_5_to_1,sv->set_info.indicator_duty);
			system_boil_off(sv);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			wait_timer_s=0;
			return;
		}
		led_set_animation(animation_nixie_tube,animation_6_to_2,sv->set_info.seg_duty);
		return;
	}

	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		sv->kettle_tick=sv->tick;
		system_boil_off(sv);
		if(sv->busy_mark&ENCODER_BUSY)
		{
			wait_timer_s=0;
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
		}
		else
		{
			wait_timer_s++;
			led_set_animation(animation_nixie_tube,animation_6,sv->set_info.seg_duty);
		}
		display_update_music_type(sv->set_music_type);
	}
	

	key_value=crontab_kettle_get_key_value();
	switch(key_value)
	{
		case KEY_VAL_START:
			if(sv->set_music_type!=sv->set_info.music_type)
			{
				sv->set_info.music_type=sv->set_music_type;
				flash_write_set_info(&sv->set_info);
			}
			led_set_animation(animation_music_led,animation_4_to_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_3_to_2,sv->set_info.indicator_duty);
			led_set_animation(animation_lighting,animation_3_to_2,sv->set_info.ring_duty);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_setting_lighting_duty;
			sv->encoder_priority=encoder_ring_duty;
			if(sv->set_info.ring_duty>(LED_MAX_DUTY*2/3))
			{
				sv->set_ring_duty=3;
			}
			else if(sv->set_info.ring_duty>(LED_MAX_DUTY/3))
			{
				sv->set_ring_duty=2;
			}
			else
			{
				sv->set_ring_duty=1;
			}
			system_boil_off(sv);
			wait_timer_s=0;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			break;
		default:break;
	}
}

static void run_kettle_setting_lighting_duty(system_variate_t *sv)
{
	static uint16_t wait_timer_s=0;
	key_value_e key_value;
	uint8_t ring_duty;
	
	if(wait_timer_s>MAIN_TIME_60S)
	{
		if(animation_2==led_get_animation(animation_nixie_tube))
		{
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
			led_set_animation(animation_brightness_led,animation_5_to_1,sv->set_info.indicator_duty);
			system_boil_off(sv);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			wait_timer_s=0;
			return;
		}
		led_set_animation(animation_nixie_tube,animation_6_to_2,sv->set_info.seg_duty);
		return;
	}

	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		sv->kettle_tick=sv->tick;
		system_boil_off(sv);
		if(sv->busy_mark&ENCODER_BUSY)
		{
			wait_timer_s=0;
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			if(sv->set_ring_duty==3)
			{
				ring_duty=LED_MAX_DUTY;
			}
			else if(sv->set_ring_duty==2)
			{
				ring_duty=LED_MAX_DUTY*2/3;
			}
			else
			{
				ring_duty=LED_MAX_DUTY/3;
			}
			led_set_animation(animation_lighting,animation_2,ring_duty);
		}
		else
		{
			wait_timer_s++;
			led_set_animation(animation_nixie_tube,animation_6,sv->set_info.seg_duty);
		}
		display_update_music_type(sv->set_ring_duty);
	}
	

	key_value=crontab_kettle_get_key_value();
	switch(key_value)
	{
		case KEY_VAL_START:
			if(sv->set_info.ring_duty>(LED_MAX_DUTY*2/3))
			{
				ring_duty=3;
			}
			else if(sv->set_info.ring_duty>(LED_MAX_DUTY/3))
			{
				ring_duty=2;
			}
			else
			{
				ring_duty=1;
			}
			if(ring_duty!=sv->set_ring_duty)
			{
				if(sv->set_ring_duty==3)
				{
					sv->set_info.ring_duty=LED_MAX_DUTY;
				}
				else if(sv->set_ring_duty==2)
				{
					sv->set_info.ring_duty=LED_MAX_DUTY*2/3;
				}
				else
				{
					sv->set_info.ring_duty=LED_MAX_DUTY/3;
				}
				flash_write_set_info(&sv->set_info);
			}
			led_set_animation(animation_brightness_led,animation_4_to_1,sv->set_info.indicator_duty);
			led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_setting_unit;
			sv->encoder_priority=encoder_temperature_unit;
			sv->set_temperature_unit=sv->set_info.temperature_unit;
			system_boil_off(sv);
			wait_timer_s=0;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			break;
		default:break;
	}
}

static void run_kettle_setting_unit(system_variate_t *sv)
{
	static uint16_t wait_timer_s=0;
	key_value_e key_value;

	if(wait_timer_s>MAIN_TIME_60S)
	{
		if(animation_2==led_get_animation(animation_nixie_tube))
		{
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			system_boil_off(sv);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			wait_timer_s=0;
			return;
		}
		led_set_animation(animation_nixie_tube,animation_6_to_2,sv->set_info.seg_duty);
		return;
	}

	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		sv->kettle_tick=sv->tick;
		system_boil_off(sv);
		if(sv->busy_mark&ENCODER_BUSY)
		{
			wait_timer_s=0;
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
		}
		else
		{
			wait_timer_s++;
			led_set_animation(animation_nixie_tube,animation_6,sv->set_info.seg_duty);
		}
		display_update_temperature_unit(sv->set_temperature_unit);
	}

	key_value=crontab_kettle_get_key_value();
	switch(key_value)
	{
		case KEY_VAL_START:
			if(sv->set_temperature_unit!=sv->set_info.temperature_unit)
			{
				sv->set_info.temperature_unit=sv->set_temperature_unit;
				if(sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)
				{
					sv->set_info.temperature=sv->set_fahrenheit_temperature;
				}
				else
				{
					sv->set_info.temperature=sv->set_temperature;
				}
				flash_write_set_info(&sv->set_info);
			}
			led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			ENCODER_OFF();
			system_boil_off(sv);
			wait_timer_s=0;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			break;
		default:break;
	}
}

static void run_kettle_error(system_variate_t *sv)
{
	static uint16_t wait_timer_s=0;
	static uint16_t error_u01_002_release_delay=0;
	key_value_e key_value;
	uint16_t wait_max_time=MAIN_TIME_5MIN;

	if(CRONTAB_TIME_100MS<=(sv->tick-sv->kettle_tick))
	{
		sv->kettle_tick=sv->tick;
		system_boil_off(sv);
		if(sv->error_mark&(ERROR_U01))
		{
			wait_max_time=MAIN_TIME_60S;
			display_update_error_u01_u02(sv->set_info.temperature_unit);
			if(sv->busy_mark&KETTLE_BUSY)
			{
				if(animation_2==led_get_animation(animation_nixie_tube))
				{
					led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
					sv->kettle_last_mode=sv->kettle_mode;
					sv->kettle_mode=kettle_power_down;
					sv->busy_mark|=POWER_DOWN_BUSY;
					sv->error_mark=0;
					wait_timer_s=0;
					return;
				}
				else
				{
					led_set_animation(animation_nixie_tube,animation_11_to_2,sv->set_info.seg_duty);
				}
			}
		}

		if(sv->error_mark&ERROR_U02)
		{
			wait_max_time=MAIN_TIME_60S;
			display_update_error_u01_u02(sv->set_info.temperature_unit);
			if((sv->busy_mark&KETTLE_BUSY)==0)
			{
				if((animation_2==led_get_animation(animation_nixie_tube)&&(animation_2==led_get_animation(animation_lighting))))
				{
					led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
					led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
					sv->kettle_last_mode=sv->kettle_mode;
					sv->kettle_mode=kettle_power_down;
					sv->busy_mark|=POWER_DOWN_BUSY;
					sv->error_mark=0;
					wait_timer_s=0;
					return;
				}
				else
				{
					led_set_animation(animation_nixie_tube,animation_11_to_2,sv->set_info.seg_duty);
					led_set_animation(animation_lighting,animation_6_to_2,sv->set_info.ring_duty);
				}
			}

			if(sv->error_mark&ERROR_U02_001)
			{
				if(sv->temperature<MAX_TEMPERATURE)
				{
					error_u01_002_release_delay++;
					if(error_u01_002_release_delay>MAIN_TIME_1S)
					{
						if((animation_2==led_get_animation(animation_nixie_tube)&&(animation_2==led_get_animation(animation_lighting))))
						{
							led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
							led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
							sv->kettle_last_mode=sv->kettle_mode;
							sv->kettle_mode=kettle_power_down;
							sv->busy_mark|=POWER_DOWN_BUSY;
							sv->error_mark=0;
							wait_timer_s=0;
							return;
						}
						else
						{
							led_set_animation(animation_nixie_tube,animation_11_to_2,sv->set_info.seg_duty);
							led_set_animation(animation_lighting,animation_6_to_2,sv->set_info.ring_duty);
						}
					}
				}
				else
				{
					error_u01_002_release_delay=0;
				}
			}
			else
			{
				error_u01_002_release_delay=0;
			}
		}

		if(sv->error_mark&ERROR_U03)
		{
			display_update_set_temperature((sv->set_info.temperature_unit&TEMP_UNIT_TYPE_FAHRNHEIT)?sv->set_fahrenheit_temperature:sv->set_temperature,sv->set_info.temperature_unit);	
			if((sv->busy_mark&KETTLE_BUSY)==0)
			{
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
				sv->kettle_last_mode=sv->kettle_mode;
				sv->kettle_mode=kettle_power_down;
				sv->busy_mark|=POWER_DOWN_BUSY;
				sv->error_mark=0;
				wait_timer_s=0;
			}

			if(led_get_animation(animation_nixie_tube)==animation_1)
			{
				ENCODER_ON();
				sv->kettle_mode=sv->kettle_last_mode;
				if(sv->kettle_mode==kettle_before_keep_warm)
				{
					led_set_animation(animation_keep_led,animation_3_to_2,sv->set_info.indicator_duty);
				}
				sv->kettle_last_mode=kettle_error;
				sv->error_mark=0;
				wait_timer_s=0;
			}
		}
		
		if(sv->error_mark&ERROR_E01)
		{
			display_update_error(1);
		}
		if(sv->error_mark&(ERROR_E02_001|ERROR_E02_002))
		{
			display_update_error(2);
		}
		if(sv->error_mark&ERROR_E03)
		{
			display_update_error(3);
		}
		if(sv->error_mark&ERROR_E04)
		{
			display_update_error(4);
		}
		
		wait_timer_s++;
		if(wait_timer_s>wait_max_time)
		{
			if(sv->error_mark&(ERROR_U01))
			{
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			}
			else
			{
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
			}
			system_boil_off(sv);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			sv->error_mark=0;
			wait_timer_s=0;
		}
	}
	key_value=crontab_kettle_get_key_value();
	switch(key_value)
	{
		case KEY_VAL_START:
			if(sv->error_mark&(ERROR_U01))
			{
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
			}
			else
			{
				led_set_animation(animation_nixie_tube,animation_5_to_1,sv->set_info.seg_duty);
				led_set_animation(animation_lighting,animation_5_to_1,sv->set_info.ring_duty);
			}
			system_boil_off(sv);
			voice_play(sv->set_info.music_type,voice3);
			sv->kettle_last_mode=sv->kettle_mode;
			sv->kettle_mode=kettle_power_down;
			sv->busy_mark|=POWER_DOWN_BUSY;
			sv->kettle_tick=sv->tick;
			sv->error_mark=0;
			wait_timer_s=0;
			break;
		default:break;
	}
}

static void run_kettle_set_pid_duty(system_variate_t *sv)
{
	static uint32_t pid_tick=0;
	
	if(sv->boil_pid.set_duty!=0)
	{
		if(pid_tick!=sv->tick)
		{
			pid_tick=sv->tick;
			if(sv->boil_pid.boil_pid_count>sv->boil_pid.set_duty)
			{
				sv->boil_mode=boil_pause;
			}
			else
			{
				sv->boil_mode=boil_on;
			}
			

			sv->boil_pid.boil_pid_count++;
			if(sv->boil_pid.boil_pid_count>=BOIL_PID_PREIOD)
			{
				sv->boil_pid.boil_pid_count=0;
			}
		}
	}
}

static void system_run_kettle(system_variate_t *sv)
{
	//sv->kettle_mode=kettle_boiled_water;
	switch(sv->kettle_mode)
	{
		case kettle_power_down:run_kettle_power_down(sv);break;
		case kettle_before_boiling:run_kettle_before_boiling(sv);break;
		case kettle_boiling_water:run_kettle_boiling_water(sv);break;
		case kettle_boiled_water:run_kettle_boiled_water(sv);break;
		case kettle_before_keep_warm:run_kettle_before_boiling(sv);break;
		case kettle_keep_warm_boiling_water:run_kettle_boiling_water(sv);break;
		case kettle_keep_warm_boiled_water:run_kettle_boiled_water(sv);break;
		case kettle_keep_warm:run_kettle_keep_warm(sv);break;
		case kettle_keep_warm_time_out:run_kettle_keep_warm_time_out(sv);break;
		case kettle_setting_music_type:run_kettle_setting_music_type(sv);break;
		case kettle_setting_lighting_duty:run_kettle_setting_lighting_duty(sv);break;
		case kettle_setting_unit:run_kettle_setting_unit(sv);break;
		case kettle_error:run_kettle_error(sv);break;
		default:sv->kettle_mode=kettle_power_down;break;
	}
	
	run_kettle_set_pid_duty(sv);
}



static void system_get_adc(system_variate_t *sv)
{
	int16_t i;
	static uint16_t sum_count=0;
	static uint32_t temperature_sum=0;
	uint32_t adcv=0;

	temperature_sum+=(ADC1->ADDR5&0xFFFF);
	sum_count++;
	if(sum_count>=1024)
	{
		adcv=(temperature_sum>>10);
		sv->temperature_adc=adcv;
		sum_count=0;
		temperature_sum=0;
		for(i=0;i<NTC_TABLE_SIZE;i++)
		{
			if(adcv>ntc_table[i])
			{
				break;
			}
		}
		if(i==0)
		{
			sv->temperature_no_filter=-4000;
		}
		else if(i==NTC_TABLE_SIZE)
		{
			sv->temperature_no_filter=14200;
		}
		else
		{
			sv->temperature_no_filter=(i-40)*100+(100-(adcv-ntc_table[i+1])*100/(ntc_table[i]-ntc_table[i+1]));
		}
	}
}

static void system_get_temperature(system_variate_t *sv)
{
#define TEMPERATURE_ACHE_SIZE		10
	static uint32_t tick=0;
	static int16_t temperature_ache[TEMPERATURE_ACHE_SIZE];
	uint8_t i=0;
	int32_t temperature_sum=0;

	if((sv->tick-tick)>=CRONTAB_TIME_100MS)
	{
		tick=sv->tick;

		for(i=0;i<(TEMPERATURE_ACHE_SIZE-1);i++)
		{
			temperature_ache[i]=temperature_ache[i+1];
			temperature_sum+=temperature_ache[i];
		}
		temperature_sum+=sv->temperature_no_filter;
		temperature_ache[TEMPERATURE_ACHE_SIZE-1]=sv->temperature_no_filter;
		sv->temperature=temperature_sum/=10;
		if(sv->set_temperature==100)
		{
			sv->temperature-=150;
		}
		sv->show_temperature=sv->temperature/100;
	}
}

static void system_run_self_check(system_variate_t *sv)
{
	key_value_e key_value;
	uint8_t i;

#ifndef TEST_SELF	
	sv->kettle_tick=sv->tick;
	while((START_KEY_GPIO->IDR&START_KEY_PIN)==0)
	{
		if(CRONTAB_TIME_2S<(sv->tick-sv->kettle_tick))
		{
			return;
		}
	}
	sv->kettle_tick=sv->tick;
	while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
	key_value=crontab_kettle_get_key_value();
	sv->kettle_tick=sv->tick;
	while(CRONTAB_TIME_5S>(sv->tick-sv->kettle_tick))
	{
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			break;
		}
		else if (KEY_VAL_START==key_value)
		{
			return;
		}
	}
	if(CRONTAB_TIME_5S<=(sv->tick-sv->kettle_tick))
	{
		return;
	}
	while(CRONTAB_TIME_5S>(sv->tick-sv->kettle_tick))
	{
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_START==key_value)
		{
			break;
		}
		else if (KEY_VAL_KEEP==key_value)
		{
			return;
		}
	}
	if(CRONTAB_TIME_5S<=(sv->tick-sv->kettle_tick))
	{
		return;
	}
	while(CRONTAB_TIME_5S>(sv->tick-sv->kettle_tick))
	{
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_START==key_value)
		{
			break;
		}
		else if (KEY_VAL_KEEP==key_value)
		{
			return;
		}
	}
	if(CRONTAB_TIME_5S<=(sv->tick-sv->kettle_tick))
	{
		return;
	}
#endif
	flash_reset_epprom_set_info(&sv->set_info);
	flash_reset_epprom_run_info(&sv->run_info);
	display_all_on();
	led_set_animation(animation_lighting,animation_2,sv->set_info.ring_duty);
	led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
	led_set_animation(animation_music_led,animation_2,sv->set_info.indicator_duty);
	led_set_animation(animation_brightness_led,animation_2,sv->set_info.indicator_duty);
	led_set_animation(animation_keep_led,animation_2,sv->set_info.indicator_duty);
	sv->kettle_tick=sv->tick;
	while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
	voice_play(sv->set_info.music_type,voice1);
	sv->kettle_tick=sv->tick;
	while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
			sv->kettle_tick=sv->tick;
			sv->boil_mode=boil_on;
			while(CRONTAB_TIME_3S>=(sv->tick-sv->kettle_tick));
			{
				led_run_animation();
			}
			sv->boil_mode=boil_off;
			if((sv->relay_error1_count>MAX_RELAY_ERROR_COUNT)||(sv->ac_lost_tick>=8)||
				(sv->relay_error2_count>MAX_RELAY_ERROR_COUNT))
			{
				led_set_animation(animation_lighting,animation_11,sv->set_info.ring_duty);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_brightness_led,animation_11,sv->set_info.indicator_duty);
				led_set_animation(animation_keep_led,animation_11,sv->set_info.indicator_duty);
				led_set_animation(animation_music_led,animation_11,sv->set_info.indicator_duty);
				for(i=0;i<3;i++)
				{
					sv->kettle_tick=sv->tick;
					while(CRONTAB_TIME_800MS>(sv->tick-sv->kettle_tick))
					{
						led_run_animation();
					}
					voice_play(sv->set_info.music_type,voice7);
					sv->kettle_tick=sv->tick;
				}
				key_value=crontab_kettle_get_key_value();
				goto FCT_END;
			}
			else
			{
				voice_play(sv->set_info.music_type,voice2);
				display_show_pass();
				led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			}
			break;
		}
		key_value=crontab_kettle_get_key_value();
	}
	
	
	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_12,sv->set_info.ring_duty);
			ENCODER_ON();
			sv->encoder_priority=encoder_fct_ccw;
			sv->fct_encoder_count=0;
			while(sv->fct_encoder_count<MAX_FCT_ENCODER_COUNT)
			{
				led_run_animation();
				if(sv->fct_encoder_count>=5)
				{
					for(i=0;i<(sv->fct_encoder_count-4);i++)
					{
						led_animation_set_single_brightness(sv->set_info.ring_duty,MAX_FCT_ENCODER_COUNT-4-i);
					}
				}
				if(sv->fct_encoder_count>=4)
				{
					led_animation_set_single_brightness(sv->set_info.ring_duty,0);
				}
				if(sv->fct_encoder_count>=3)
				{
					led_set_animation(animation_music_led,animation_2,sv->set_info.indicator_duty);
				}
				if(sv->fct_encoder_count>=2)
				{
					led_set_animation(animation_brightness_led,animation_2,sv->set_info.indicator_duty);
				}
				if(sv->fct_encoder_count>=1)
				{
					led_set_animation(animation_keep_led,animation_2,sv->set_info.indicator_duty);
				}
			}
			ENCODER_OFF();
			for(i=0;i<=AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(sv->set_info.ring_duty,i);
			}
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			key_value=crontab_kettle_get_key_value();
			break;
		}
	}

	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			ENCODER_ON();
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_12,sv->set_info.ring_duty);
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
			for(i=0;i<AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(0,i);
			}
			ENCODER_ON();
			sv->encoder_priority=encoder_fct_cw;
			sv->fct_encoder_count=0;
			while(sv->fct_encoder_count<MAX_FCT_ENCODER_COUNT)
			{
				led_run_animation();
				if(sv->fct_encoder_count>=4)
				{
					for(i=0;i<(sv->fct_encoder_count-3);i++)
					{
						led_animation_set_single_brightness(sv->set_info.ring_duty,i);
					}
				}
				if(sv->fct_encoder_count>=3)
				{
					led_set_animation(animation_music_led,animation_2,sv->set_info.indicator_duty);
				}
				if(sv->fct_encoder_count>=2)
				{
					led_set_animation(animation_brightness_led,animation_2,sv->set_info.indicator_duty);
				}
				if(sv->fct_encoder_count>=1)
				{
					led_set_animation(animation_keep_led,animation_2,sv->set_info.indicator_duty);
				}
			}
			ENCODER_OFF();
			for(i=0;i<=AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(sv->set_info.ring_duty,i);
			}
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			key_value=crontab_kettle_get_key_value();
			break;
		}
	}

	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
			sv->boil_mode=boil_on;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>=(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			sv->boil_mode=boil_off;
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>=(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice2);
			break;
		}
	}


	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);

			display_update_number(1111);
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			display_update_number(2222);
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			display_update_number(3333);
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			display_update_number(4444);
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			display_update_number(5555);
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			display_all_on();
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				led_run_animation();
			}
			voice_play(sv->set_info.music_type,voice2);
			key_value=crontab_kettle_get_key_value();
			break;
		}
	}

	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_2S>(sv->tick-sv->kettle_tick))
			{
				system_get_adc(&system_variate);
			}
			if((sv->temperature_no_filter>0)&&(sv->temperature_no_filter<10000))
			{
				voice_play(sv->set_info.music_type,voice2);
				display_show_pass();
				led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			}
			else
			{
				led_set_animation(animation_lighting,animation_11,sv->set_info.ring_duty);
				led_set_animation(animation_nixie_tube,animation_11,sv->set_info.seg_duty);
				led_set_animation(animation_brightness_led,animation_11,sv->set_info.indicator_duty);
				led_set_animation(animation_keep_led,animation_11,sv->set_info.indicator_duty);
				led_set_animation(animation_music_led,animation_11,sv->set_info.indicator_duty);
				for(i=0;i<3;i++)
				{
					sv->kettle_tick=sv->tick;
					while(CRONTAB_TIME_1S>(sv->tick-sv->kettle_tick))
					{
						led_run_animation();
					}
					voice_play(sv->set_info.music_type,voice7);
					sv->kettle_tick=sv->tick;
				}
				goto FCT_END;
			}
			break;
		}
	}

	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_START==key_value)
		{
			flash_reset_epprom_run_info(&sv->run_info);
			flash_reset_epprom_set_info(&sv->set_info);
			led_set_animation(animation_lighting,animation_1,sv->set_info.ring_duty);
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
			display_show_fin();
			voice_play(sv->set_info.music_type,voice3);
			break;
		}
	}
FCT_END:	
	while (1)
	{
		led_run_animation();
	}
	
}
#endif

#ifdef TEST_FCT
static void system_run_fct_check(system_variate_t *sv)
{
	key_value_e key_value;
	uint8_t i;

	while(1)
	{
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_START==key_value)
		{
			break;
		}
	}

	display_all_on();
	led_set_animation(animation_lighting,animation_2,sv->set_info.ring_duty);
	led_set_animation(animation_nixie_tube,animation_2,sv->set_info.seg_duty);
	led_set_animation(animation_music_led,animation_2,sv->set_info.indicator_duty);
	led_set_animation(animation_brightness_led,animation_2,sv->set_info.indicator_duty);
	led_set_animation(animation_keep_led,animation_2,sv->set_info.indicator_duty);
	sv->kettle_tick=sv->tick;
	while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
	voice_play(sv->set_info.music_type,voice1);
	sv->kettle_tick=sv->tick;
	while(CRONTAB_TIME_80MS>(sv->tick-sv->kettle_tick));
	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_12,sv->set_info.ring_duty);
			for(i=0;i<=AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(0,i);
			}
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
			ENCODER_ON();
			sv->encoder_priority=encoder_fct_ccw;
			sv->fct_encoder_count=0;
			while(sv->fct_encoder_count<MAX_FCT_ENCODER_COUNT)
			{
				led_run_animation();
				if(sv->fct_encoder_count>=2)
				{
					for(i=0;i<(sv->fct_encoder_count-1);i++)
					{
						led_animation_set_single_brightness(sv->set_info.ring_duty,AVAILABLE_PORTS-1-i);
					}
				}
				if(sv->fct_encoder_count>=1)
				{
					led_animation_set_single_brightness(sv->set_info.ring_duty,0);
				}
				if(sv->fct_encoder_count>=AVAILABLE_PORTS)
				{
					break;
				}
			}
			ENCODER_OFF();
			for(i=0;i<=AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(sv->set_info.ring_duty,i);
			}
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			key_value=crontab_kettle_get_key_value();
			break;
		}
	}

	while (1)
	{
		led_run_animation();
		key_value=crontab_kettle_get_key_value();
		if(KEY_VAL_KEEP==key_value)
		{
			ENCODER_ON();
			led_set_animation(animation_nixie_tube,animation_1,sv->set_info.seg_duty);
			led_set_animation(animation_lighting,animation_12,sv->set_info.ring_duty);
			led_set_animation(animation_music_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_brightness_led,animation_1,sv->set_info.indicator_duty);
			led_set_animation(animation_keep_led,animation_1,sv->set_info.indicator_duty);
			for(i=0;i<AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(0,i);
			}
			ENCODER_ON();
			sv->encoder_priority=encoder_fct_cw;
			sv->fct_encoder_count=0;
			while(sv->fct_encoder_count<MAX_FCT_ENCODER_COUNT)
			{
				led_run_animation();
				for(i=0;i<sv->fct_encoder_count;i++)
				{
					led_animation_set_single_brightness(sv->set_info.ring_duty,i);
				}

				if(sv->fct_encoder_count>=AVAILABLE_PORTS)
				{
					break;
				}
				
			}
			ENCODER_OFF();
			for(i=0;i<=AVAILABLE_PORTS;i++)
			{
				led_animation_set_single_brightness(sv->set_info.ring_duty,i);
			}
			sv->kettle_tick=sv->tick;
			while(CRONTAB_TIME_200MS>(sv->tick-sv->kettle_tick));
			voice_play(sv->set_info.music_type,voice2);
			key_value=crontab_kettle_get_key_value();
			break;
		}
	}

	while (1)
	{
		led_run_animation();
	}
}
#endif


/***********************************************************************************************************************
  * @brief  This function is main entrance
  * @note   main
  * @param  none
  * @retval none
  *********************************************************************************************************************/
int main(void)
{
	gpio_config();

	delay_init();
	
	adc_config();
	
	relay_pwm_config();
	
	timer3_pwm_config();
	
	voice_config();
	
	led_config();
	
    display_config();
	
	trace_init();

#ifdef USING_EEPROM	
	flash_read_epprom_set_info(&system_variate.set_info);
	flash_read_epprom_run_info(&system_variate.run_info);
#else
	flash_read_set_info(&system_variate.set_info);
	flash_read_run_info(&system_variate.run_info);
#endif
	
	system_variate.flash_run_time_s=system_variate.run_info.run_time_s;
	system_variate.tick=system_variate.run_info.run_time_s*100;
	system_variate.temperature=25;
	system_variate.boil_timer=TIM16;
	
#ifdef TEST_RELAY	
	led_set_animation(animation_nixie_tube,animation_2,100);
	display_update_number(0);
#else

#ifdef TEST_FCT
	#warning "TEST_FCT"
	system_run_fct_check(&system_variate);
#endif

#ifdef TEST_SELF
	#warning "TEST_SELF"
	system_run_self_check(&system_variate);
#else
	if((START_KEY_GPIO->IDR&START_KEY_PIN)==0)
	{
		system_variate.tick_1ms=0;
		while(1)
		{
			if(START_KEY_GPIO->IDR&START_KEY_PIN)
			{
				break;
			}
		}
		if(system_variate.tick_1ms>5)
		{
			system_run_self_check(&system_variate);
		}
	}
#endif
#endif	
    while (1)
    {
#ifdef TEST_RELAY
		system_check_relay(&system_variate);
#else		
		system_get_adc(&system_variate);
		system_get_temperature(&system_variate);
		system_run_kettle(&system_variate);
		trace_handle();
#endif
		led_run_animation();
		
    }
}



