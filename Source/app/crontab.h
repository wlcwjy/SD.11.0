#ifndef _CRONTAB_H_
#define _CRONTAB_H_
#include "hal_conf.h"
#include "system.h"

#define CRONTAB_TIME_10MS					1
#define CRONTAB_TIME_20MS					2
#define CRONTAB_TIME_40MS					4
#define CRONTAB_TIME_80MS					8
#define CRONTAB_TIME_100MS				10
#define CRONTAB_TIME_120MS				12
#define CRONTAB_TIME_200MS				20
#define CRONTAB_TIME_250MS				25
#define CRONTAB_TIME_400MS				40
#define CRONTAB_TIME_500MS				50
#define CRONTAB_TIME_600MS				60
#define CRONTAB_TIME_800MS				80
#define CRONTAB_TIME_1S					100
#define CRONTAB_TIME_1_3S					130
#define CRONTAB_TIME_1_5S					150
#define CRONTAB_TIME_2S					200
#define CRONTAB_TIME_3S					300
#define CRONTAB_TIME_5S					500
#define CRONTAB_TIME_6S					600
#define CRONTAB_TIME_10S					1000
#define CRONTAB_TIME_20S					2000
#define CRONTAB_TIME_30S					3000
#define CRONTAB_TIME_120S					12000
#define CRONTAB_TIME_3600S				360000
#define CRONTAB_TIME_7200S				720000
#define CRONTAB_TIME_6MIN					36000
#define CRONTAB_TIME_10MIN				60000
#define CRONTAB_TIME_30MIN				180000
#define CRONTAB_TIME_1H					360000

typedef struct 
{
	uint8_t key_mark;
	
	key_value_e kettle_key_value;
	key_value_e timer_key_value;
	key_value_e temperature_unit_value;
	
	uint16_t kettle_remove_delay;
	uint16_t kettle_in_delay;
	uint16_t key_pressed_delay;

	uint16_t keep_key_count;
	uint16_t start_key_count;
	
}crontab_variate_t;

extern void crontab_config(void);
extern key_value_e crontab_timer_get_key_value(void);
extern key_value_e crontab_kettle_get_key_value(void);
extern key_value_e crontab_temperature_unit_get_key_value(void);
#endif
