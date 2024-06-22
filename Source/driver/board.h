#ifndef _BOARD_H_
#define _BOARD_H_

#include <stdint.h>
#include "system.h"

#define RELAY_GPIO							GPIOA
#define RELAY_PIN							GPIO_Pin_2

#define MUSIC_LED_GPIO						GPIOA
#define MUSIC_LED_PIN						GPIO_Pin_3

#define BRIGHTNESS_LED_GPIO					GPIOA
#define BRIGHTNESS_LED_PIN					GPIO_Pin_4

#define ZERO_CROSS_GPIO						GPIOA
#define ZERO_CROSS_PIN						GPIO_Pin_6

#define TIM14_PERIOD_GPIO					GPIOA
#define TIM14_PERIOD_PIN					GPIO_Pin_7

#define ENCODER_A_GPIO						GPIOA
#define ENCODER_A_PIN				 		GPIO_Pin_8

#define LED_1_2_GPIO						GPIOA
#define LED_1_2_PIN				 			GPIO_Pin_15

#define VOICE_RESET_GPIO					GPIOD
#define VOICE_RESET_PIN						GPIO_Pin_2

#define START_KEY_GPIO						GPIOB
#define START_KEY_PIN				 		GPIO_Pin_0

#define TEST_GPIO							GPIOB
#define TEST_PIN				 			GPIO_Pin_1

#define TEST1_GPIO							GPIOB
#define TEST1_PIN				 			GPIO_Pin_2

#define LED_3_4_GPIO						GPIOB
#define LED_3_4_PIN				 			GPIO_Pin_3

#define LED_5_6_GPIO						GPIOB
#define LED_5_6_PIN				 			GPIO_Pin_4

#define LED_7_8_GPIO						GPIOB
#define LED_7_8_PIN				 			GPIO_Pin_5

#define LED_9_10_GPIO						GPIOB
#define LED_9_10_PIN				 		GPIO_Pin_6

#define LED_11_12_GPIO						GPIOB
#define LED_11_12_PIN				 		GPIO_Pin_7

#define AIP33620_SCLK_GPIO					GPIOB
#define AIP33620_SCLK_PIN				 	GPIO_Pin_10

#define AIP33620_DIN_GPIO					GPIOB
#define AIP33620_DIN_PIN				 	GPIO_Pin_11

#define ENCODER_B_GPIO						GPIOB
#define ENCODER_B_PIN				 		GPIO_Pin_15

#define KEEP_LED_GPIO						GPIOB
#define KEEP_LED_PIN						GPIO_Pin_13

#define KEEP_KEY_GPIO						GPIOB
#define KEEP_KEY_PIN						GPIO_Pin_14

#define RELAY_FAIL_GPIO						GPIOC
#define RELAY_FAIL_PIN						GPIO_Pin_13

#define LED_13_14_GPIO						GPIOC
#define LED_13_14_PIN				 		GPIO_Pin_14

#define LED_15_16_GPIO						GPIOC
#define LED_15_16_PIN				 		GPIO_Pin_15

#define LED_17_18_GPIO						GPIOD
#define LED_17_18_PIN				 		GPIO_Pin_0

#define LED_19_20_GPIO						GPIOD
#define LED_19_20_PIN				 		GPIO_Pin_1

#define	ENCODER_LINE_B						EXTI_Line15

#define RELAY_ON()							RELAY_GPIO->BSRR=RELAY_PIN
#define RELAY_OFF()							RELAY_GPIO->BRR=RELAY_PIN

#define ENCODER_OFF()						EXTI->IMR&=~ENCODER_LINE_B;EXTI->FTSR&=~ENCODER_LINE_B;EXTI->RTSR&=~ENCODER_LINE_B
#define ENCODER_ON()						EXTI->IMR|=ENCODER_LINE_B;EXTI->FTSR|=ENCODER_LINE_B;EXTI->RTSR|=ENCODER_LINE_B

#define HB_PULS_ON()						TIM16->CCMR1|=0x0060
#define HB_PULS_OFF()						TIM16->CCMR1&=~0x0020

#define NTC_TABLE_SIZE						183

#define MAX_CRONTAB_TIMER					5999
#define NTC_ERROR_TEMPERATURE				12500
#define HEATING_AGING_TEMPERATURE			500
#define MAX_TEMPERATURE						11000
#define DRY_FIRING_TEMPERATURE				3000
#define MAX_SET_FAHRENHEIT_TEMPERATURE		212
#define MAX_SET_TEMPERATURE					100
#define MIN_SET_FAHRENHEIT_TEMPERATURE		122
#define MIN_SET_TEMPERATURE					50
#define KETTLY_REMOVE_TEMPERATURE			(-1500)
#define KETTLY_IN_TEMPERATURE				(-1000)
#define KETTLY_TCOMP						150

#define MAX_RELAY_ERROR_COUNT				125

#define BOIL_TIMER_PREIOD 					6014
#define BOIL_PID_PREIOD 					1500
#define MIN_BOIL_PID_PREIOD 				350

#define BOIL_PID_KP							5000
#define BOIL_PID_KI							600
#define BOIL_PID_KD							60
#define BOIL_PID_MAX_STEP					(BOIL_PID_PREIOD>>2)

#define MAX_FCT_ENCODER_COUNT			13


#define SWAP_UINT16(x)					((x & 0x00FF ) <<8) | ((x & 0xFF00)>>8);



typedef struct 
{
	uint16_t temperature;	
}adc_channel_t;

static const uint16_t adc_number=sizeof(adc_channel_t)/sizeof(uint16_t);	

extern adc_channel_t act; 

extern const uint16_t ntc_table[];
extern const uint16_t crc16_table[];



extern void relay_pwm_config(void);
extern void timer3_pwm_config(void);

extern void system_clock_config(void);
extern void gpio_config(void);
extern void adc_config(void);

extern void Error_Handler(void);
#endif
