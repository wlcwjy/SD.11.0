

#ifndef _LED_ANIMATION_H_
#define _LED_ANIMATION_H_

#include "system.h"
#include "led.h"

#define FLUCTUATION_ANIM_MS_DURATION 	500 // ���˥�`������g�� [msec]

#define PWM_INTERVAL_US 					(1000) // PWM�������󥿩`�Х� = PWM�ܲ��� 1kHz
#define PWM_INTERVAL_PERIOD 				(100) // PWM�������󥿩`�Х� = PWM�ܲ��� 1kHz
#define MAX_DUTY 							(PWM_INTERVAL_US) // ���duty
#define LED_CHANNELS 						(32) // LED�Υ���ͥ���


/*--------------------------------------------------
  LED���˥�`������x (FEB��led.h�����äƤ���)
 --------------------------------------------------*/
#define MABIKI_PATTERN 	(4) // LED�g�����ѥ��`�� (1:�ʤ�30�ư׳�, 2:15�ư׳�, 3:15�Ƴ�, 4:10�Ƴ�, 5:8�Ƴ�)

#if (MABIKI_PATTERN == 1)
 #define AVAILABLE_PORTS (30) // �Є���LED�ݩ`����
#elif (MABIKI_PATTERN == 2)
 #define AVAILABLE_PORTS (15) // �Є���LED�ݩ`����
#elif (MABIKI_PATTERN == 3)
 #define AVAILABLE_PORTS (15) // �Є���LED�ݩ`����
#elif (MABIKI_PATTERN == 4)
 #define AVAILABLE_PORTS (10) // �Є���LED�ݩ`����
#elif (MABIKI_PATTERN == 5)
 #define AVAILABLE_PORTS (8) // �Є���LED�ݩ`����
#endif

/**
* @enum RptType_
* ���˥�`�������R�귵���N�e
*/
typedef enum {
  LED_ANIMATION_CONTINUE, //!< �ΤΥ��ƥåפ��M��
  LED_ANIMATION_REPEAT,  //!< ָ�������R�귵��
  LED_ANIMATION_END    //!< ���˥�`�����K��
} RptType_;

/**
* @enum ���˥�`�����N�e
*/
typedef enum {
  ANIME_TYPE_LINEAR,
  ANIME_TYPE_SLOW_IN_FAST_OUT,
  ANIME_TYPE_FAST_IN_SLOW_OUT,
  ANIME_TYPE_STATIC,
  NUMOF_ANIM_TYPE
} AnimeType;

/**
* @struct LEDAnimStep
* ���˥�`�����1���ƥå׷֤����
*/
typedef struct {
  void*   nextTo_;   //!< �ΤΥ��ƥåפؤΥݥ��� @attention ����Υ��ƥåפϱؤ�NULL�ˤ��뤳��
  int16_t  dutyTo_;   //!< ���_������duty
  int16_t  msDuration_; //!< ���_������r�g[msec]
  AnimeType type_;    //!< ���`�֥�����
  RptType_ rpt_type_;  //!< �R�귵���N�e
  uint8_t  rpt_cnt_;  //!< �R�귵������ @details 1~255, 0�������R�귵��: rpt_type_��LED_ANIMATION_REPEAT�r�Τ��Є�
  uint8_t  reserve_;  //!< ��s�I��(���饤�����{��)
} LEDAnimStep;

/**
* @struct LEDAnimeProperty
* ���˥�`�����ץ�ѥƥ������
*/
typedef struct {
  const LEDAnimStep*  pFirstStep_;  //!< ���˥�`���������^���ƥå�
  const LEDAnimStep*  pCurrStep_;  //!< ���˥�`�����άF�ڄ����Х��ƥå�
  unsigned long    startTime_;  //!< ���˥�`�����Υ��ƥå��_ʼ�r�g
  int16_t       duty_;     //!< �F�ڤ�duty��
  int16_t       show_duty_;     //!< �F�ڤ�duty��
  int16_t       dutyFrom_;   //!< ���˥�`�����Υ��ƥå��_ʼ�r���duty��
  bool         bRepeat_;   //!< �R�귵�����ƥåפ���ä��H�˜ʂ�g���ɤ������Єe����
  bool         isAnimating_; //!< ���˥�`������Ф��ɤ���
  uint8_t       rpt_cnt_;   //!< �R�귵���л���
} LEDAnimeProperty;

/*--------------------------------------------------
  ���˥�`�����I��(FEB��ValueAnimator�����äƤ���)
 --------------------------------------------------*/
/**
* ���˥�`�����Y��
*/
typedef struct {
  int32_t value_;
  bool  bFinished_;
} ValueAnimatorResult;

/**
* �ݩ`�ȷN�e
*/
typedef enum {
 TYPEA,
 TYPEA_,
 TYPEB,
 TYPEB_,
 TYPEC,
 TYPEC_,
 TYPED,
 TYPED_,
 TYPEE,
 TYPEE_,
 TYPEF,
 TYPEF_,
 TYPEG,
 TYPEG_,
 TYPEH,
 TYPEH_,
 TYPEI,
 TYPEI_,
 TYPEJ,
 TYPEJ_,
 NUMOF_PORTTYPE,
} PortType;

/**
* �ݩ`���������
*/
typedef struct 
{
	int port_;    //!< �ݩ`�ȷ���
	PortType type_;  //!< �ݩ`�ȷN�e
} PortInfo;

/**
* �ݩ`���������
*/
typedef struct 
{
	GPIO_TypeDef* port_;    
	uint16_t pin;  
} gpio_info_t;

/**
* LED���ӾA����Ƥ���ݩ`�ȶ��x
*/
extern void led_animation_set_brightness(uint8_t brightness);
extern void led_animation_loop(void);
extern void led_animation_pwm(void) ;
extern uint8_t led_animation_get_duty(void);
extern void led_animation_set_single_brightness(uint8_t brightness,uint8_t n);
extern void led_animation13_loop(animation_info_t *ait) ;
#endif
