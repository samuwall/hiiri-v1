/**********************************************************************************
 ** file            : spi.c
 ** description     : generic SPIM operations (nRF52)
 **
 **********************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "device.h"
#include "spi.h"

uint8_t spi_transfer(SPIM_T *SPIMx, uint8_t data) {
    
    volatile uint8_t tx[1] = {data};
    volatile uint8_t rx[1];

    SPIMx->EVENTS_END = 0;
    SPIMx->TXD.PTR    = (uint32_t) tx;
    SPIMx->TXD.MAXCNT = 1;
    SPIMx->RXD.PTR    = (uint32_t) rx;
    SPIMx->RXD.MAXCNT = 1;

    SPIMx->TASKS_START = 1;
    while (!(SPIMx->EVENTS_END));

    return rx[0];
}