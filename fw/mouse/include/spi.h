/**********************************************************************************
 ** file            : spi.h
 ** description     : generic SPIM operations (nRF52)
 **
 **********************************************************************************/

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "device.h"

uint8_t spi_transfer(SPIM_T *SPIMx, uint8_t data);

#endif