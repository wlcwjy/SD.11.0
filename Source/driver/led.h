/* app_main.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com/wlcwjy@163.com
 * 日期：2024年4月15日
 * led驱动程序头文件
 */
#ifndef _LED_H_
#define _LED_H_
#include "hal_conf.h"


#define LED_MAX_DUTY						100

typedef enum
{
	animation_lighting=0,
	animation_nixie_tube=1,
	animation_music_led=2,
	animation_brightness_led=3,
	animation_keep_led=4,
    animation_type_size=5
}animation_type_e;


typedef enum
{
	animation_1=0,
	animation_2=1,
	animation_3_to_2=2,
	animation_4_to_1=3,
	animation_5_to_1=4,
	animation_6=5,
	animation_7=6,
	animation_8_to_2=7,
	animation_9_to_5=8,
	animation_10=9,
	animation_11_to_1=10,
	animation_6_to_1=11,
    animation_3_to_7=12,
	animation_4_to_10=13,
	animation_11=14,
	animation_9=15,
	animation_6_to_2=16,
	animation_10_to_1=17,
	animation_12=18,
	animation_11_to_2=19,
	animation_13=20,
}animation_mode_e;

typedef enum
{
	direction_up=0,
	direction_down=1,
	direction_hold_high=2,
}direction_e;

typedef struct 
{
	animation_mode_e animation_mode;
	animation_mode_e trace_animation_mode;
    direction_e  direction;
	uint8_t duty_count;
    uint8_t duty;
	uint8_t trace_duty;
    uint8_t max_duty;
	uint8_t run_times;
	uint8_t step;
    uint16_t base_time;
    uint32_t timer_count;
	uint8_t duty_count1;
	uint8_t duty_count2;

    void (*update_duty)(uint8_t duty);
}animation_info_t;

 
extern void led_config(void);
extern unsigned long led_millis(void);
extern void led_run_animation(void);
extern void led_set_animation(animation_type_e type,animation_mode_e mode,uint16_t max_duty);
extern animation_mode_e led_get_animation(animation_type_e type);
extern void led_set_trace_animation(animation_type_e type,uint16_t max_duty);
#endif




