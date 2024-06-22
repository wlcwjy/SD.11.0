#ifndef _AIP33620_H_
#define _AIP33620_H_

#include "system.h"

#define DISPLAY_WAIT							20000

typedef enum
{
	display_off=0,
	display_on=1,
}display_mode_e;


extern void display_config(void);
extern void display_update_error(uint8_t e);
extern void display_update_temperature_unit(uint8_t unit);


extern void display_update_duty(uint8_t duty);
extern void display_dma_handler(void);
extern void display_update_current_temperature(int16_t t,uint8_t unit);
extern void display_update_set_temperature(int16_t t,uint8_t unit);
extern void display_update_music_type(uint8_t type);
extern void display_update_error_u01_u02(uint8_t unit);
extern void display_show_pass(void);
extern void display_show_fin(void);
extern void display_all_on(void);
void display_update_number(uint16_t t);

#endif
