
#include "led_animation.h"
#include "led.h"
#include <stdlib.h> // �����׼��
#include "slow_action.h"

/*--------------------------------------------------
��餮���˥�`������x
 --------------------------------------------------*/
// �O�ӥѥ��`�󤫤������ʤ�����
#define FLUC_PATTERN 							(3) // ��餮�ѥ��`�� (��������ʽ)0:3�����ġ�1:һ���w�Ф�, 2:4���w�Ф�, (����ľ��ʽ)3:�O��, 4:һ���w�Ф�, 5:4���w�Ф� 

#define FLUC_ANIM_STEPS 						(3) // ��餮���˥�`�����Υ��ƥå���
#define CONF_VALUE_ANIMATOR_DURATION_MAX 	(INT16_MAX) // ���˥�`�������������
#define CONF_VALUE_ANIMATOR_RATIO				PWM_INTERVAL_US // ���˥�`�����ζ��A�α���

#define PERCENT_TO_DECIMAL 					0.01 // �ѩ`����Ȃ�����С���ˉ�Q
#define FLUCTUATION_ANIM_MAX_DUTY 			MAX_DUTY 
#define FLUCTUATION_VECTOR 					0 // ��餮���˥�`��������������
//#define FLUCTUATION_ANIM_MIN_DUTY_MAIN (FLUCTUATION_ANIM_MAX_DUTY*10/10) // ��餮����x��
#define FLUCTUATION_ANIM_MIN_DUTY_SUB (FLUCTUATION_ANIM_MAX_DUTY*1/10) // ��餮��С�x��
#define FLUCTUATION_ANIM_MIN_DUTY_MAIN 		FLUCTUATION_ANIM_MAX_DUTY // ��餮����x��
//#define FLUCTUATION_ANIM_MIN_DUTY_SUB 		(0) // ��餮��С�x��
#define FLUCTUATION_ANIM_DIFF_DUTY (FLUCTUATION_ANIM_MAX_DUTY*8/10) // ��餮�Ή�ӷ�
//#define FLUCTUATION_ANIM_DIFF_DUTY 			(FLUCTUATION_ANIM_MAX_DUTY*10/10) // ��餮�Ή�ӷ�
#define ANIM_TERM 								{NULL, 0, 0, ANIME_TYPE_LINEAR, LED_ANIMATION_REPEAT, 0, 0},
#define FLUCTUATION_ANIM_STEP(dutyTo) \
{ \
{ NULL, dutyTo, FLUCTUATION_ANIM_MS_DURATION, ANIME_TYPE_LINEAR, LED_ANIMATION_CONTINUE, 0, 0}, \
ANIM_TERM \
},

#define FLUCTUATION_ANIM_CENTER_DUTY 		(FLUCTUATION_ANIM_MAX_DUTY*6/10)
static LEDAnimStep s_fluctuation_anim_[LED_CHANNELS][FLUC_ANIM_STEPS] = {
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)

FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)

FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
FLUCTUATION_ANIM_STEP(FLUCTUATION_ANIM_CENTER_DUTY)
};

#if (FLUC_PATTERN <= 3)
 #if (MABIKI_PATTERN == 1)
  PortInfo led_ports[AVAILABLE_PORTS] = {
   { 22, TYPEA },
   { 23, TYPEA_},
   { 24, TYPEA },
   { 25, TYPEA_},
   { 26, TYPEA },
   { 27, TYPEA_},
   { 28, TYPEB },
   { 29, TYPEB_},
   { 30, TYPEB },
   { 31, TYPEB_},
   { 32, TYPEB },
   { 33, TYPEB_},
   { 34, TYPEC },
   { 35, TYPEC_},
   { 36, TYPEC },
   { 37, TYPEC_},
   { 38, TYPEC },
   { 39, TYPEC_},
   { 40, TYPED },
   { 41, TYPED_},
   { 42, TYPED },
   { 43, TYPED_},
   { 44, TYPED },
   { 45, TYPED_},
   { 46, TYPEE },
   { 47, TYPEE_},
   { 48, TYPEE },
   { 49, TYPEE_},
   { 50, TYPEE },
   { 51, TYPEE_},
  };
 #elif (MABIKI_PATTERN == 2)
  PortInfo led_ports[AVAILABLE_PORTS] = {
   { 22, TYPEA },
   { 23, TYPEA_},
//   { 24, TYPEA },
//   { 25, TYPEA_},
   { 26, TYPEA },
   { 27, TYPEA_},
//   { 28, TYPEB },
//   { 29, TYPEB_},
   { 30, TYPEA },
   { 31, TYPEA_},
//   { 32, TYPEB },
//   { 33, TYPEB_},
   { 34, TYPEB },
   { 35, TYPEB_},
//   { 36, TYPEC },
//   { 37, TYPEC_},
   { 38, TYPEB },
   { 39, TYPEB_},
//   { 40, TYPED },
//   { 41, TYPED_},
   { 42, TYPEB },
   { 43, TYPEB_},
//   { 44, TYPED },
//   { 45, TYPED_},
   { 46, TYPEC },
   { 47, TYPEC_},
//   { 48, TYPEE },
//   { 49, TYPEE_},
   { 50, TYPEC },
//   { 51, TYPEC_},
  };
 #elif (MABIKI_PATTERN == 3)
  PortInfo led_ports[AVAILABLE_PORTS] = {
//   { 22, TYPEA },
   { 23, TYPEA_},
//   { 24, TYPEA },
   { 25, TYPEA_},
//   { 26, TYPEA },
   { 27, TYPEA_},
//   { 28, TYPEB },
   { 29, TYPEB_},
//   { 30, TYPEB },
   { 31, TYPEB_},
//   { 32, TYPEB },
   { 33, TYPEB_},
//   { 34, TYPEC },
   { 35, TYPEC_},
//   { 36, TYPEC },
   { 37, TYPEC_},
//   { 38, TYPEC },
   { 39, TYPEC_},
//   { 40, TYPED },
   { 41, TYPED_},
//   { 42, TYPED },
   { 43, TYPED_},
//   { 44, TYPED },
   { 45, TYPED_},
//   { 46, TYPEE },
   { 47, TYPEE_},
//   { 48, TYPEE },
   { 49, TYPEE_},
//   { 50, TYPEE },
   { 51, TYPEE_},
  };
 #elif (MABIKI_PATTERN == 4)
gpio_info_t  gpio_info[AVAILABLE_PORTS] = 
{
   {GPIOA, GPIO_Pin_15},
   {GPIOB, GPIO_Pin_3},
   {GPIOB, GPIO_Pin_4,},
   {GPIOB, GPIO_Pin_5,},
   {GPIOB, GPIO_Pin_6, },
   {GPIOB, GPIO_Pin_7, },
   {GPIOC, GPIO_Pin_14, },
   {GPIOC, GPIO_Pin_15, },
   {GPIOD, GPIO_Pin_0, },
   {GPIOD, GPIO_Pin_1, },
};

PortInfo led_ports[AVAILABLE_PORTS] = {
//	 { 22, TYPEA },
   { 23, TYPEA_},
//   { 24, TYPEA },
   { 25, TYPEB_},
//   { 26, TYPEA },
//   { 27, TYPEA_},
//   { 28, TYPEB },
   { 29, TYPEC_},
//   { 30, TYPEB },
   { 31, TYPED_},
//   { 32, TYPEB },
//   { 33, TYPEB_},
//   { 34, TYPEC },
   { 35, TYPEE_},
//   { 36, TYPEC },
   { 37, TYPEF_},
//   { 38, TYPEC },
//   { 39, TYPEC_},
//   { 40, TYPED },
   { 41, TYPEG_},
//   { 42, TYPED },
   { 43, TYPEH_},
//   { 44, TYPED },
//   { 45, TYPED_},
//   { 46, TYPEE },
   { 47, TYPEI_},
//   { 48, TYPEE },
   { 49, TYPEJ_},
  };
 #elif (MABIKI_PATTERN == 5)
  PortInfo led_ports[AVAILABLE_PORTS] = {
//   { 22, TYPEA },
   { 23, TYPEA_},
//   { 24, TYPEA },
//   { 25, TYPEA_},
//   { 26, TYPEA },
   { 27, TYPEA_},
//   { 28, TYPEB },
//   { 29, TYPEB_},
//   { 30, TYPEB },
   { 31, TYPEB_},
//   { 32, TYPEB },
//   { 33, TYPEB_},
//   { 34, TYPEC },
   { 35, TYPEC_},
//   { 36, TYPEC },
//   { 37, TYPEC_},
//   { 38, TYPEC },
    { 39, TYPEC_},
//   { 40, TYPED },
//   { 41, TYPED_},
//   { 42, TYPED },
   { 43, TYPED_},
//   { 44, TYPED },
//   { 45, TYPED_},
//   { 46, TYPEE },
   { 47, TYPEE_},
//   { 48, TYPEE },
//   { 49, TYPEE_},
//   { 50, TYPEE },
   { 51, TYPEE_},
  };
 #endif
/*
#elif (FLUC_PATTERN == 4)
 #if (MABIKI_PATTERN == 0)
  PortInfo led_ports[AVAILABLE_PORTS] = {
   { 22, TYPEA },
   { 23, TYPEA_},
   { 24, TYPEB },
   { 25, TYPEB_},
   { 26, TYPEA },
   { 27, TYPEA_},
   { 28, TYPEB },
   { 29, TYPEB_},
   { 30, TYPEA },
   { 31, TYPEA_},
   { 32, TYPEB },
   { 33, TYPEB_},
   { 34, TYPEC },
   { 35, TYPEC_},
   { 36, TYPED },
   { 37, TYPED_},
   { 38, TYPEC },
   { 39, TYPEC_},
   { 40, TYPED },
   { 41, TYPED_},
   { 42, TYPEC },
   { 43, TYPEC_},
   { 44, TYPED },
   { 45, TYPED_},
   { 46, TYPEE },
   { 47, TYPEE_},
   { 48, TYPEF },
   { 49, TYPEF_},
   { 50, TYPEE },
   { 51, TYPEE_},
  };
 #elif (MABIKI_PATTERN == 1)
  PortInfo led_ports[AVAILABLE_PORTS] = {
   { 22, TYPEA },
   { 23, TYPEA_},
//   { 24, TYPEB },
//   { 25, TYPEB_},
   { 26, TYPEB },
   { 27, TYPEB_},
//   { 28, TYPEB },
//   { 29, TYPEB_},
   { 30, TYPEA },
   { 31, TYPEA_},
//   { 32, TYPEB },
//   { 33, TYPEB_},
   { 34, TYPEB },
   { 35, TYPEB_},
//   { 36, TYPED },
//   { 37, TYPED_},
   { 38, TYPEC },
   { 39, TYPEC_},
//   { 40, TYPED },
//   { 41, TYPED_},
   { 42, TYPED },
   { 43, TYPED_},
//   { 44, TYPED },
//   { 45, TYPED_},
   { 46, TYPEC },
   { 47, TYPEC_},
//   { 48, TYPEF },
//   { 49, TYPEF_},
   { 50, TYPED },
//   { 51, TYPED_},
  };
 #endif

#elif (FLUC_PATTERN == 5)
 #if (MABIKI_PATTERN == 0)
  PortInfo led_ports[AVAILABLE_PORTS] = {
   { 22, TYPEA },
   { 23, TYPEA_},
   { 24, TYPEB },
   { 25, TYPEB_},
   { 26, TYPEC },
   { 27, TYPEC_},
   { 28, TYPED },
   { 29, TYPED_},
   { 30, TYPEE },
   { 31, TYPEE_},
   { 32, TYPEA },
   { 33, TYPEA_},
   { 34, TYPEB },
   { 35, TYPEB_},
   { 36, TYPEC },
   { 37, TYPEC_},
   { 38, TYPED },
   { 39, TYPED_},
   { 40, TYPEE },
   { 41, TYPEE_},
   { 42, TYPEA },
   { 43, TYPEA_},
   { 44, TYPEB },
   { 45, TYPEB_},
   { 46, TYPEC },
   { 47, TYPEC_},
   { 48, TYPED },
   { 49, TYPED_},
   { 50, TYPEE },
   { 51, TYPEE_},
  };
 #elif (MABIKI_PATTERN == 1)
  PortInfo led_ports[AVAILABLE_PORTS] = {
   { 22, TYPEA },
   { 23, TYPEA_},
//   { 24, TYPEB },
//   { 25, TYPEB_},
   { 26, TYPEB },
   { 27, TYPEB_},
//   { 28, TYPED },
//   { 29, TYPED_},
   { 30, TYPEC },
   { 31, TYPEC_},
//   { 32, TYPEA },
//   { 33, TYPEA_},
   { 34, TYPED },
   { 35, TYPED_},
//   { 36, TYPEC },
//   { 37, TYPEC_},
   { 38, TYPEE },
   { 39, TYPEE_},
//   { 40, TYPEE },
//   { 41, TYPEE_},
   { 42, TYPEA },
   { 43, TYPEA_},
//   { 44, TYPEB },
//   { 45, TYPEB_},
   { 46, TYPEC },
   { 47, TYPEC_},
//   { 48, TYPED },
//   { 49, TYPED_},
   { 50, TYPED },
//   { 51, TYPED_},
  };
 #endif 
*/
#endif

/*--------------------------------------------------
 �����`�Х����
 --------------------------------------------------*/
unsigned long prev_tick = 0; // ǰ�ؤ�OS�r�g
unsigned long prev_elapsed = 0; // ǰ�ؤ�OS�r�g����νU�^�r�g
unsigned long prev_fluc = 0; // ǰ�ؤΤ�餮���˥�`�����Ӌ��r�g

LEDAnimeProperty led_anime[LED_CHANNELS] = { 0 }; // LED���˥�`��������

static bool led_port_sts[AVAILABLE_PORTS] = { false };

// ����constrain����
static int constrain(int x, int min, int max) 
{
    if (x < min) {
        return min;
    } else if (x > max) {
        return max;
    } else {
        return x;
    }
}

static int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) 
{
	if(out_max>out_min)
	{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}
	else
	{
		return out_min-(x - in_min) * (out_min - out_max) / (in_max - in_min);
	}
    
}

/**
* ���˥�`������_ʼ�r�Υץ��ѥƥ���������
*/
static void LED_PlayOneAnimationProperty(LEDAnimeProperty* p, uint16_t duty) 
{
	p->pCurrStep_ = p->pFirstStep_;
	p->startTime_ = led_millis();
	p->bRepeat_  = false;
	p->isAnimating_ = true;
	p->dutyFrom_ = duty;
	p->rpt_cnt_  = 0;
}

/**
* ָ���������˥�`������_ʼ
*/
static void LED_PlayOneAnimation(LEDAnimeProperty* pProperty, const LEDAnimStep* pAnim) \
{
	pProperty->pFirstStep_ = pAnim;
	LED_PlayOneAnimationProperty(pProperty, pProperty->duty_);
}

///**
//* ָ���������˥�`������_ʼ(�_ʼ�r���duty���Ĥ�)
//*/
//static void LED_PlayOneAnimationFromDuty(LEDAnimeProperty* pProperty, const LEDAnimStep* pAnim, uint16_t fromDuty) 
//{
//	pProperty->pFirstStep_ = pAnim;
//	LED_PlayOneAnimationProperty(pProperty, fromDuty);
//}
uint16_t show_duty[AVAILABLE_PORTS]={0};
uint16_t from_duty[AVAILABLE_PORTS]={0};
uint16_t to_duty[AVAILABLE_PORTS]={0};
/**
* ��餮��duty����Ӌ�㤷���O��
*/
static void LedMgr_SetFluctuationDuty(void) 
{
  static uint8_t direction=0;
	int16_t diff=0;
	LEDAnimStep *p = NULL;
#if (FLUC_PATTERN >= 3)
	int16_t diffs[NUMOF_PORTTYPE];
	int16_t *q = diffs;
#endif
// uint16_t min_duty_main = FLUCTUATION_ANIM_MIN_DUTY_MAIN + (uint16_t)((MAX_DUTY - FLUCTUATION_ANIM_MIN_DUTY_MAIN) * (PERCENT_TO_DECIMAL * attenuator_));
// uint16_t min_duty_sub = FLUCTUATION_ANIM_MIN_DUTY_SUB + (uint16_t)((MAX_DUTY - FLUCTUATION_ANIM_MIN_DUTY_SUB) * (PERCENT_TO_DECIMAL * self->attenuator_));
// uint16_t fluc_limit = FLUCTUATION_ANIM_MAX_DUTY + (uint16_t)((LANTERN_MAX_DUTY - FLUCTUATION_ANIM_MAX_DUTY) * (PERCENT_TO_DECIMAL * self->attenuator_));
	uint16_t min_duty_main = FLUCTUATION_ANIM_MIN_DUTY_MAIN;
	uint16_t min_duty_sub = FLUCTUATION_ANIM_MIN_DUTY_SUB;
	uint16_t fluc_limit = FLUCTUATION_ANIM_MAX_DUTY;
	if (min_duty_main > MAX_DUTY) min_duty_main = MAX_DUTY;
	if (min_duty_sub > MAX_DUTY) min_duty_sub = MAX_DUTY;
	if (fluc_limit > MAX_DUTY) fluc_limit = MAX_DUTY;

//	randomSeed(micros()); 
  srand(led_millis());
#if (FLUC_PATTERN >= 3)
 
	for (uint8_t i = 0; i < NUMOF_PORTTYPE; ++i, ++q) 
	{
		//*q = led_random(-1 * FLUCTUATION_ANIM_DIFF_DUTY, FLUCTUATION_ANIM_DIFF_DUTY);
		*q=(rand()%(FLUCTUATION_ANIM_DIFF_DUTY<<1)-FLUCTUATION_ANIM_DIFF_DUTY);
	}
#endif

	for (uint8_t idx = 0; idx < AVAILABLE_PORTS; ++idx) 
	{
		// ��餮�Ή�����������ǛQ��

#if (FLUC_PATTERN == 0)
		if (!idx || (((idx + 1) % 6) == 0)) {
		 //diff = random(-1 * FLUCTUATION_ANIM_DIFF_DUTY, FLUCTUATION_ANIM_DIFF_DUTY);
     	diff=(rand()%(FLUCTUATION_ANIM_DIFF_DUTY<<1)-FLUCTUATION_ANIM_DIFF_DUTY);
		}
#elif (FLUC_PATTERN == 1)
		if (!idx || (((idx + 1) % 4) == 0)) {
		 //diff = random(-1 * FLUCTUATION_ANIM_DIFF_DUTY, FLUCTUATION_ANIM_DIFF_DUTY);
     diff=(rand()%(FLUCTUATION_ANIM_DIFF_DUTY<<1)-FLUCTUATION_ANIM_DIFF_DUTY);
		}
#elif (FLUC_PATTERN == 2) 
		//if (!idx || (((idx + 1) % 10) == 0)) {
		 if (!idx || (((idx + 1) % 10) == 0)) {
		  //diff = random(-1 * FLUCTUATION_ANIM_DIFF_DUTY, FLUCTUATION_ANIM_DIFF_DUTY);
      diff=(rand()%(FLUCTUATION_ANIM_DIFF_DUTY<<1)-FLUCTUATION_ANIM_DIFF_DUTY);
		 }
#else
		diff = diffs[led_ports[idx].type_];
#endif

	  // CANDLE_LED_DUMMY_CH0��٩`����CH13��duty_to��Q������
	  // CH0  : �g����diff���������
	  // CH1-3 : ���¤��줿CH0��٩`����diff�����֡�ɢ��Ф餻��
    
		p = &s_fluctuation_anim_[idx][FLUCTUATION_VECTOR]; 
    p->dutyTo_ = diff + s_fluctuation_anim_[idx][FLUCTUATION_VECTOR].dutyTo_;
    // if(direction==0)
    // {
    //    p->dutyTo_ = diff + min_duty_main;
    // }
    // else
    // {
    //     p->dutyTo_ = diff + min_duty_sub;
    // }
		p->dutyTo_ = constrain(p->dutyTo_, min_duty_sub, fluc_limit);
    from_duty[idx]=led_anime[idx].duty_;
    to_duty[idx]=p->dutyTo_;
	//  if (led_anime[idx].duty_) {
		LED_PlayOneAnimation(&led_anime[idx], p);
	//  } else {
	//   LED_PlayOneAnimationFromDuty(&led_anime[idx], p, MAX_DUTY);
	//  }
	}

   if(direction==0)
    {
       direction=1;
    }
    else
    {
        direction=0;
    }
}


/**
* ��˥��ʥ��˥�`�����
*/
int32_t CalcLinear(int16_t msPassed, int16_t msDuration) 
{
	if ((msDuration <= 0) || (msPassed < 0))
	{
		return 0;
	}
	/* return cur_norm_time */
	int32_t value = CONF_VALUE_ANIMATOR_RATIO;
	value *= msPassed;
	value /= msDuration;
	return value;
}

/**
* ����
*/
int32_t CalcStatic(int16_t msPassed, int16_t msDuration)
{
	return CONF_VALUE_ANIMATOR_RATIO;
}

/** 
* ���˥�`�����N�e�v���Ʃ`�֥�
*/
typedef int32_t (*FuncCalcCurve)(int16_t msPassed, int16_t msDuration);
static const FuncCalcCurve s_curveTbl[NUMOF_ANIM_TYPE] = 
{
	CalcLinear, // ANIME_TYPE_LINEAR
	// CalcSlowInFastOut, // ANIME_TYPE_SLOW_IN_FAST_OUT,
	// CalcFastInSlowOut, // ANIME_TYPE_FAST_IN_SLOW_OUT, 
	NULL,
	NULL,
	CalcStatic, // ANIME_TYPE_STATIC,
};

/**
* ���˥�`����󂎤�Ӌ��
*/
ValueAnimatorResult ValueAnimator_Calc(int16_t msPassed, int16_t msDuration, AnimeType type) 
{
	ValueAnimatorResult ret;
	msDuration = constrain(msDuration, 0, CONF_VALUE_ANIMATOR_DURATION_MAX);
	msPassed = constrain(msPassed, 0, msDuration);
	ret.bFinished_ = msDuration<=msPassed;
	ret.value_ = s_curveTbl[type](msPassed, msDuration);
	return ret;
}

/*--------------------------------------------------
LED���˥�`�����I��
--------------------------------------------------*/
/**
* ���˥�`�������Ȥ��M���
* @param [in] pAnim ���˥�`�����ץ��ѥƥ�
*
* @retval TRUE ���˥�`�����Ͼ@�A��
* @retval FALSE ���˥�`�����ϽK��ä�
*
* @remarks
* rpt_type_��REPEAT�Έ��ϡ��Ф�rpt������0�Ǥʤ���С�self->pCurrStep_ = self->pFirstStep_�Ȥ���TRUE�򷵤���
* 0�Έ��Ϥϡ��g���˴ΤΥ��ƥåפؤ���TRUE�򷵤�
* rpt_type_��REPEAT�ʤ������ͬ�r��rpt_cnt_��0���ä������ä˥�`�פ���褦�ˤ���
*/
static bool LED_AheadAnimation(LEDAnimeProperty* pAnim) 
{

	pAnim->startTime_ += pAnim->pCurrStep_->msDuration_;
	pAnim->pCurrStep_++;

	if (pAnim->pCurrStep_->rpt_type_ == LED_ANIMATION_REPEAT) 
	{
		// �R�귵��
		if (pAnim->bRepeat_ == false) 
		{
			pAnim->bRepeat_ = true;
			pAnim->rpt_cnt_ = pAnim->pCurrStep_->rpt_cnt_;
			pAnim->pCurrStep_ = pAnim->pFirstStep_;
		} 
		else 
		{
			if (!pAnim->pCurrStep_->rpt_cnt_)
			{
				// ���ä��R�귵��
				pAnim->pCurrStep_ = pAnim->pFirstStep_;
			} 
			else 
			{
				if (pAnim->rpt_cnt_ > 1) 
				{
					// �R�귵���Ф����Ф������1���󤭤�
					pAnim->pCurrStep_ = pAnim->pFirstStep_;
					pAnim->rpt_cnt_--;
				} 
				else
				{
					// �R�귵���ФǲФ������1
					pAnim->bRepeat_ = false;
					pAnim->pCurrStep_++;// 1���ƥå��M��
				}
			}
		}
	} 
	else if (pAnim->pCurrStep_->rpt_type_ == LED_ANIMATION_END)
	{
		// ���˥�`�����K��
		if (pAnim->pCurrStep_->nextTo_ != NULL) 
		{
			pAnim->pFirstStep_ = pAnim->pCurrStep_->nextTo_;
			pAnim->pCurrStep_ = pAnim->pCurrStep_->nextTo_;
		} 
		else 
		{
			return false;
		}
	}
	pAnim->dutyFrom_ = pAnim->duty_;
	return true;
}

// ָ���������˥�`�����ֹͣ
void LED_StopOneAnimation(LEDAnimeProperty* p) 
{
	p->isAnimating_ = false;
	p->bRepeat_ = false;
	p->dutyFrom_ = p->duty_;
	p->rpt_cnt_ = 0;
}

void LED_Update(void) 
{
	LEDAnimeProperty* pAnim = &led_anime[0];

	for (uint8_t i = 0; i < AVAILABLE_PORTS; ++i, ++pAnim) 
	{
		const LEDAnimStep* animeCurr = pAnim->pCurrStep_;
		if (!animeCurr) continue; 

		ValueAnimatorResult ret = ValueAnimator_Calc((led_millis() - pAnim->startTime_), animeCurr->msDuration_, (AnimeType)animeCurr->type_);
		pAnim->duty_ = (int16_t)map(ret.value_, 0, CONF_VALUE_ANIMATOR_RATIO, pAnim->dutyFrom_, animeCurr->dutyTo_);
		pAnim->show_duty_=(pAnim->duty_/10);
    show_duty[i]=pAnim->duty_;
		if (ret.bFinished_) 
		{
			if (! LED_AheadAnimation(pAnim)) 
			{
				LED_StopOneAnimation(pAnim);
			}
		}
	}
}

/*--------------------------------------------------
  �ᥤ��I��
 --------------------------------------------------*/
/**
* GPIO�ݩ`�Ȥ�ʹ�ä�PWM�I��
*/
void led_animation_pwm(void) 
{
//	unsigned long tick = micros();
	static uint8_t elapsed = 0;
	uint8_t i;

//	if (elapsed < 50) return; // 50��sec���˄I������

	elapsed++;
	if (elapsed >= PWM_INTERVAL_PERIOD) 
	{
		elapsed = 0;
	}

	LEDAnimeProperty* p = led_anime;
	PortInfo *q = led_ports;
	bool *r = led_port_sts;
	for ( i = 0; i < AVAILABLE_PORTS; ++i, ++p, ++q, ++r) 
	{
		if (elapsed < p->show_duty_) 
		{
//			if (!*r) 
//			{
				gpio_info[i].port_->BSRR=gpio_info[i].pin;
//				*r = true;
//			}
		} 
		else 
		{
//			if (*r) 
//			{
				gpio_info[i].port_->BRR=gpio_info[i].pin;
//				*r = false;
//			}
		}
	}
}

void led_animation_loop(void) 
{
    static unsigned long prev_looptime = 0;
    unsigned long looptime = led_millis();


    if ((looptime - prev_fluc) >= FLUCTUATION_ANIM_MS_DURATION) 
    {
        LedMgr_SetFluctuationDuty();
        prev_fluc = looptime;
    }

  
    if ((looptime - prev_looptime) >= 30) 
    {
        LED_Update();
        prev_looptime = looptime;
    }
	
    //DoPWM();
    // looptime = millis() - looptime;
    // Serial.println(String(looptime, DEC));
}


void led_animation_set_brightness(uint8_t brightness)
{
	uint16_t i=0;
// PWM�����I������ (1��sec����������Ƥ��뤬��Arduino MEGA�������޽�ˤ��16��sec��)
	LEDAnimeProperty* p = led_anime;

	for ( i = 0; i < AVAILABLE_PORTS; ++i, ++p) 
	{
		p->show_duty_=brightness;
	}
}

void led_animation_set_single_brightness(uint8_t brightness,uint8_t n)
{
    if(n>=AVAILABLE_PORTS)
    {
        return;
    }

    LEDAnimeProperty* p = led_anime;

    p[n].show_duty_=brightness;
 
}

uint8_t led_animation_get_duty(void)
{
	uint16_t i=0;
	uint8_t duty=0;
	
	LEDAnimeProperty* p = led_anime;
	duty=p->show_duty_;
	for ( i = 0; i < AVAILABLE_PORTS; ++i, ++p) 
	{
		if(p->show_duty_<=duty)
		{
			duty=p->show_duty_;
		}
    
	}
	return duty;
}

//#define TYPEA


void led_animation13_loop(animation_info_t *ait) 
{
    uint8_t duty[3];
    uint8_t i=0;
    LEDAnimeProperty* p = led_anime;

	if(ait->timer_count<ait->base_time)
	{
		return;
	}

#ifdef TYPEA
    ait->timer_count=0;
    ait->duty_count++;
    ait->duty_count1++;
    ait->duty_count2++;
    duty[0]=slow_ease_in_out_sine_U0_180(0,ait->max_duty,ait->duty_count);
    if(ait->duty_count1>(ait->max_duty*2/3))
    {
        if(ait->duty_count1<=ait->max_duty)
        {
            duty[1]=slow_ease_in_out_sine_U0_180(0,ait->max_duty,ait->duty_count1);
        }
        else
        {
            duty[1]=slow_ease_in_out_sine_U180_360(ait->max_duty,0,ait->max_duty*2-ait->duty_count1);
        }
    }
    else
    {
        duty[1]=0;
    }
    if(ait->duty_count2>(ait->max_duty*4/3))
    {
       duty[2]=slow_ease_in_out_sine_U180_360(ait->max_duty,0,ait->max_duty*2-ait->duty_count2);
    }
    else
    {
        duty[2]=0;
    }
    for(i=0;i<3;i++)
    {
        if(duty[i]>=ait->max_duty)
        {
            duty[i]=ait->max_duty;
        }
    }
    if(ait->step==0)
    {
        p[0].show_duty_=duty[0];
        p[9].show_duty_=duty[1];
        p[8].show_duty_=duty[2];
    }
    else if(ait->step==1)
    {
        p[1].show_duty_=duty[0];
        p[0].show_duty_=duty[1];
        p[9].show_duty_=duty[2];
    }
    else
    {
        p[ait->step].show_duty_=duty[0];
        p[ait->step-1].show_duty_=duty[1];
        p[ait->step-2].show_duty_=duty[2];
    }
    if(ait->duty_count>=(ait->max_duty*2/3))
    {
        ait->step++;
        if(ait->step>=AVAILABLE_PORTS)
        {
            ait->step=0;
        }
        ait->duty_count=0;
        if(ait->duty_count1>(ait->max_duty*2/3))
        {
            ait->duty_count1=(ait->max_duty*2/3);
        }
        if(ait->duty_count2>(ait->max_duty*4/3))
        {
            ait->duty_count2=(ait->max_duty*4/3);
        }
    }
#else
    ait->timer_count=0;
    ait->duty_count++;
    ait->duty_count1++;
    duty[0]=slow_ease_in_out_sine_U0_180(0,ait->max_duty,ait->duty_count);
    if(ait->duty_count1>ait->max_duty)
    {
        duty[1]=slow_ease_in_out_sine_U180_360(ait->max_duty,0,ait->max_duty*2-ait->duty_count1);
    }
    else
    {
        duty[1]=0;
    }
    for(i=0;i<2;i++)
    {
        if(duty[i]>=ait->max_duty)
        {
            duty[i]=ait->max_duty;
        }
    }
    if(ait->step==0)
    {
        p[0].show_duty_=duty[0];
        p[9].show_duty_=duty[1];
    }
    else
    {
        p[ait->step].show_duty_=duty[0];
        p[ait->step-1].show_duty_=duty[1];
    }
    if(ait->duty_count>=ait->max_duty)
    {
        ait->step++;
        if(ait->step>=AVAILABLE_PORTS)
        {
            ait->step=0;
        }
        ait->duty_count=0;
        if(ait->duty_count1>ait->max_duty)
        {
            ait->duty_count1=ait->max_duty;
        }
    }


#endif
}












