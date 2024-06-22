/* app_main.c
 * ���ߣ�wlc(����C)
 * ���䣺85276902@qq.com/wlcwjy@163.com
 * ���ڣ�2024��4��15��
 * �ṩ������������ͷ�ļ�
 */
#ifndef _VOICE_H_
#define _VOICE_H_
#include "hal_conf.h"

        
typedef enum 
{
	voice1=0,
	voice2=1,      
	voice3=2,		
	voice4=3,      
	voice5=4,
    voice6=5,
    voice7=6,	
}voice_number_e;

typedef enum 
{
	voice_channel1=0,
	voice_channel2=1,      
	voice_channel3=2,		
	voice_channel4=3,      
}voice_channel_e;



extern void voice_play(uint8_t index,voice_number_e music);
extern void voice_config(void);
extern void voice_dma_handler(void);
extern void voice_set_voice(voice_number_e voice);
#endif

