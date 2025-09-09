/**********************************************************************************
 ** file            : delay.c
 ** description     : delay functions (nRF52)
 **
 **********************************************************************************/

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>
#include "device.h"

void delay_us(TIMER_T *TIMERx, uint32_t us);

#endif