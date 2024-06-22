

#ifndef _LED_ANIMATION_H_
#define _LED_ANIMATION_H_

#include "system.h"
#include "led.h"

#define FLUCTUATION_ANIM_MS_DURATION 	500 // アニメーション間隔 [msec]

#define PWM_INTERVAL_US 					(1000) // PWM制御インターバル = PWM周波数 1kHz
#define PWM_INTERVAL_PERIOD 				(100) // PWM制御インターバル = PWM周波数 1kHz
#define MAX_DUTY 							(PWM_INTERVAL_US) // 最大duty
#define LED_CHANNELS 						(32) // LEDのチャネル数


/*--------------------------------------------------
  LEDアニメーション定義 (FEBのled.hからもってきた)
 --------------------------------------------------*/
#define MABIKI_PATTERN 	(4) // LED間引きパターン (1:なし30灯白橙, 2:15灯白橙, 3:15灯橙, 4:10灯橙, 5:8灯橙)

#if (MABIKI_PATTERN == 1)
 #define AVAILABLE_PORTS (30) // 有効なLEDポート数
#elif (MABIKI_PATTERN == 2)
 #define AVAILABLE_PORTS (15) // 有効なLEDポート数
#elif (MABIKI_PATTERN == 3)
 #define AVAILABLE_PORTS (15) // 有効なLEDポート数
#elif (MABIKI_PATTERN == 4)
 #define AVAILABLE_PORTS (10) // 有効なLEDポート数
#elif (MABIKI_PATTERN == 5)
 #define AVAILABLE_PORTS (8) // 有効なLEDポート数
#endif

/**
* @enum RptType_
* アニメーションの繰り返し種別
*/
typedef enum {
  LED_ANIMATION_CONTINUE, //!< 次のステップへ進む
  LED_ANIMATION_REPEAT,  //!< 指定回数繰り返し
  LED_ANIMATION_END    //!< アニメーション終了
} RptType_;

/**
* @enum アニメーション種別
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
* アニメーション1ステップ分の情報
*/
typedef struct {
  void*   nextTo_;   //!< 次のステップへのポインタ @attention 最後のステップは必ずNULLにすること
  int16_t  dutyTo_;   //!< 到達させるduty
  int16_t  msDuration_; //!< 到達させる時間[msec]
  AnimeType type_;    //!< カーブタイプ
  RptType_ rpt_type_;  //!< 繰り返し種別
  uint8_t  rpt_cnt_;  //!< 繰り返し回数 @details 1~255, 0は永久繰り返し: rpt_type_がLED_ANIMATION_REPEAT時のみ有効
  uint8_t  reserve_;  //!< 予約領域(アライメント調整)
} LEDAnimStep;

/**
* @struct LEDAnimeProperty
* アニメーションプロパティ用情報
*/
typedef struct {
  const LEDAnimStep*  pFirstStep_;  //!< アニメーションの先頭ステップ
  const LEDAnimStep*  pCurrStep_;  //!< アニメーションの現在動作中ステップ
  unsigned long    startTime_;  //!< アニメーションのステップ開始時間
  int16_t       duty_;     //!< 現在のduty値
  int16_t       show_duty_;     //!< 現在のduty値
  int16_t       dutyFrom_;   //!< アニメーションのステップ開始時点のduty値
  bool         bRepeat_;   //!< 繰り返しステップに入った際に準備済かどうかを判別する
  bool         isAnimating_; //!< アニメーション中かどうか
  uint8_t       rpt_cnt_;   //!< 繰り返し残回数
} LEDAnimeProperty;

/*--------------------------------------------------
  アニメーション処理(FEBのValueAnimatorからもってきた)
 --------------------------------------------------*/
/**
* アニメーション結果
*/
typedef struct {
  int32_t value_;
  bool  bFinished_;
} ValueAnimatorResult;

/**
* ポート種別
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
* ポート情報構造体
*/
typedef struct 
{
	int port_;    //!< ポート番号
	PortType type_;  //!< ポート種別
} PortInfo;

/**
* ポート情報構造体
*/
typedef struct 
{
	GPIO_TypeDef* port_;    
	uint16_t pin;  
} gpio_info_t;

/**
* LEDが接続されているポート定義
*/
extern void led_animation_set_brightness(uint8_t brightness);
extern void led_animation_loop(void);
extern void led_animation_pwm(void) ;
extern uint8_t led_animation_get_duty(void);
extern void led_animation_set_single_brightness(uint8_t brightness,uint8_t n);
extern void led_animation13_loop(animation_info_t *ait) ;
#endif
