/**********************************************************************************
 ** file            : delay.c
 ** description     : delay functions (nRF52)
 **
 **********************************************************************************/

#include <stdint.h>
#include "device.h"
#include "delay.h"

/* assumes timer is scaled to 1MHz
 */
void delay_us(TIMER_T *TIMERx, uint32_t us) {

    TIMERx->TASKS_STOP  = 1;
    TIMERx->TASKS_CLEAR = 1;
    TIMERx->CC[0]       = us;
    TIMERx->EVENTS_COMPARE[0] = 0;
    TIMERx->TASKS_START = 1;

    while (!(TIMERx->EVENTS_COMPARE[0]));

    TIMERx->TASKS_STOP  = 1;
    TIMERx->TASKS_CLEAR = 1;
    TIMERx->EVENTS_COMPARE[0] = 0;

}