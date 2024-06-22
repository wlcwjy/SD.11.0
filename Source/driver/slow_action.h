/* app_main.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com/wlcwjy@163.com
 * 日期：2024年5月7日
 * 提供缓动程序函数头文件
 */
#ifndef _SLOW_ACTION_H_
#define _SLOW_ACTION_H_

#include "hal_conf.h"


/** @addtogroup modules
  * @{
  */

/** @addtogroup transform
  * @{
  */
#define SIN_MASK    0x3000
#define U0_90       0x0000
#define U90_180     0x1000
#define U180_270    0x2000
#define U270_360    0x3000

/*   Function Declaration   */
extern uint16_t slow_ease_in_sine_U0_90(uint16_t start, uint16_t end, uint16_t value); 
extern uint16_t slow_ease_out_sine_U90_180(uint16_t start, uint16_t end, uint16_t value);
extern uint16_t slow_ease_in_out_sine_U0_180(uint16_t start, uint16_t end, uint16_t value);
extern uint16_t slow_ease_in_out_sine_U180_360(uint16_t start, uint16_t end, uint16_t value);
extern uint16_t slow_ease_out_circ(uint16_t start, uint16_t end, uint16_t value) ;
extern uint16_t slow_ease_out_cubic_up(uint16_t start, uint16_t end, uint16_t value) ;
extern uint16_t slow_ease_out_cubic_down(uint16_t start, uint16_t end, uint16_t value) ;
/**
  * @}
  */
/**
 * @}
 */

/** @addtogroup global_variables
  * @{
  */

/**
 * @}
 */

#endif  /* End of _SLOW_ACTION_H_ */

