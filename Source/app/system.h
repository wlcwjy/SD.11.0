#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "hal_conf.h"

#define USING_EEPROM							

#define KEY_BUSY								(1<<0)
#define KETTLE_BUSY								(1<<1)
#define ENCODER_BUSY							(1<<2)
#define POWER_DOWN_BUSY							(1<<3)
#define KEEP_WARM_BUSY							(1<<4)
#define BOILED_BUSY								(1<<5)
#define KEEP_WARN_BOILING_STOP_BUSY				(1<<6)

#define ERROR_U01								(1<<0)
#define ERROR_U02								(1<<1)
#define ERROR_U03								(1<<2)
#define ERROR_E01								(1<<3)
#define ERROR_E02_001							(1<<4)
#define ERROR_E02_002							(1<<5)
#define ERROR_E03								(1<<6)
#define ERROR_E04								(1<<7)
#define ERROR_U02_001							(1<<8)
#define ERROR_U02_002							(1<<9)

#define TEMP_UNIT_TYPE_CELSIYS					(0)
#define TEMP_UNIT_TYPE_FAHRNHEIT				(1)


#define LED_MARK_TIMER							(1<<0)
#define LED_MARK_COUNT_UP						(1<<1)
#define LED_MARK_TEMP_SET						(1<<2)
#define LED_MARK_KEEP							(1<<3)

#define SOFTWARE_MAJOR 							1
#define SOFTWARE_DEBUG							62

#define SW_VERSION								SOFTWARE_MAJOR,SOFTWARE_DEBUG

typedef enum 
{
	KEY_VAL_NULL,
	KEY_VAL_START,      
	KEY_VAL_KEEP,		

	KEY_VAL_LP_START,
	KEY_VAL_LP_KEEP,
}key_value_e;

typedef enum
{
	timer_power_down=0,
	timer_setting=1,
	timer_count_down=2,
	timer_count_up=3,
}timer_mode_e;

typedef enum
{
	kettle_power_down=0,
	kettle_before_boiling=1,
	kettle_boiling_water=2,
	kettle_boiled_water=3,
	kettle_before_keep_warm=4,
	kettle_keep_warm_boiling_water=5,
	kettle_keep_warm_boiled_water=6,
	kettle_keep_warm=7,
	kettle_keep_warm_time_out=8,
	kettle_setting_music_type=9,
	kettle_setting_lighting_duty=10,
	kettle_setting_unit=11,
	kettle_check_pcb=12,
	kettle_check_product=13,
	kettle_error=14,
}kettle_mode_e;

typedef enum
{
	control_idle=0,
	control_hb_start=2,
	control_relay_start=3,
	control_boiling=4,
}control_mode_e;

typedef enum
{
	boil_off=0,
	boil_on=1,
	boil_pause=2,
}boil_mode_e;

typedef enum
{
	encoder_temperature=0,
	encoder_music_type=1,
	encoder_ring_duty=2,
	encoder_temperature_unit=3,
	encoder_fct_cw=4,
	encoder_fct_ccw=5,
}encoder_priority_e;

typedef struct 
{
	uint32_t temperature;	
	uint32_t temperature_unit;
	uint32_t music_type;
	uint32_t led_mode;
	uint32_t pid_duty;
	uint32_t ring_duty;
	uint32_t seg_duty;
	uint32_t indicator_duty;
	uint32_t version;
	uint32_t checksum;
}set_info_t;

typedef struct 
{
	uint32_t run_time_s;	
	uint32_t start_key_count;	
	uint32_t keep_key_count;
	uint32_t remove_count;
	uint32_t boiled_count;
	uint32_t keep_count;
    uint32_t plateau_boiling_count;
	uint16_t u01_count;
	uint16_t u02_count;
	uint16_t u03_count;
	uint16_t e01_count;
	uint16_t e02_001_count;
	uint16_t e02_002_count;
	uint16_t e03_count;
	uint16_t e04_count;
	uint32_t relay1_count;
	uint32_t relay1_time_s;
	uint32_t relay2_count;
	uint32_t relay2_time_s;
	uint32_t version;
	uint32_t checksum;
}run_info_t;

typedef struct PID
{
	uint32_t kp;			//IQ16
	uint32_t ki;			//IQ16
	uint32_t kd; 			//IQ16
	int32_t limit;

	int32_t current_error;	//��ǰ����
	int32_t last_error;		//��һ������	
	int32_t previous_error;	//���ϴ�����

	uint16_t boil_pid_count;
	
	uint8_t first_in_flag;
	int16_t current_duty;
	int16_t set_duty;
}pid_t;

typedef struct 
{
	uint32_t tick_1ms;
	uint32_t tick;
	uint32_t ac_lost_tick;
	uint16_t busy_mark;
	set_info_t set_info;
	run_info_t	run_info;
	uint32_t flash_run_time_s;

	uint32_t kettle_tick;
	kettle_mode_e kettle_mode;
	kettle_mode_e kettle_last_mode;
	int16_t	 set_temperature;
	int16_t	 set_fahrenheit_temperature;
	int16_t temperature_no_filter;
	int16_t temperature;
	int16_t show_temperature;
	int16_t last_show_temperature;
	int16_t initial_temperature;
	int16_t max_boiling_temperature;
	uint16_t temperature_adc;
	control_mode_e control_mode;	
	boil_mode_e boil_mode;
	pid_t boil_pid;

	encoder_priority_e encoder_priority;
	uint16_t encoder_idle_delay;
	uint16_t encoder_voice_delay;
	uint8_t relay_error1_delay;
	uint8_t relay_error2_delay;
	uint8_t keep_warn_change_delay;
	
	uint32_t temperature_unit_tick;
	uint32_t error_tick;
	uint8_t led_mark;
	uint8_t set_music_type;
	uint8_t set_ring_duty;
	uint8_t set_temperature_unit;

	uint16_t error_mark;

	uint16_t u02_error_time_count;
	uint16_t e03_error_time_count;
	uint16_t e04_error_time_count;
	uint16_t relay_error1_count;
	uint16_t relay_error2_count;
	uint16_t plateau_boiling_count;
	uint16_t keep_warm_wait_timer_s;

	uint8_t fct_encoder_count;
	
	TIM_TypeDef* boil_timer;

}system_variate_t;

extern const uint16_t capacity_value[];
extern const uint16_t capacity_cycle[];
extern const uint16_t mode_m_capacity_factor[];


extern system_variate_t *system_get_variate(void);

#endif



































































































































































































































































































