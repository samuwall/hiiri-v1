/**********************************************************************************
 ** file            : paw3395.c
 ** description     : helper functions for paw3395 (nRF52)
 **
 **********************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "device.h"
#include "spi.h"
#include "delay.h"
#include "utils.h"
#include "paw3395.h"

static uint8_t paw_read(uint8_t addr) {

    uint8_t data;

    /* t_srr / t_srw */
    delay_us(TIMER0, 2);

    /* pull CS low: select this chip */
    P0->OUTCLR = GPIO0;

    /* send reg addr we wish to read */
    spi_transfer(SPIM0, addr);

    /* t_srad */
    delay_us(TIMER0, 2);

    /* receive reg data */
    data = spi_transfer(SPIM0, 0);

    /* pull CS high: transaction complete */
    P0->OUTSET = GPIO0;

    return data;
}

static void paw_write(uint8_t addr, uint8_t data) {

    /* t_sww / t_swr */
    delay_us(TIMER0, 5);

    /* select chip */
    P0->OUTCLR = GPIO0;

    /* send reg addr we wish to write to with MSB set (write op) */
    spi_transfer(SPIM0, addr | (1 << 7));

    /* write reg data */
    spi_transfer(SPIM0, data);

    /* pull CS high: complete */
    P0->OUTSET = GPIO0;

}

static void paw_modify(uint8_t addr, uint8_t clearmask, uint8_t setmask) {
    uint8_t reg = paw_read(addr);
    reg = (reg & ~clearmask) | (setmask);
    paw_write(addr, reg);
}

void paw_init(void) {

    /* Power On Sequence, as described in section 6.0: */

    /* step 1: wait for VDD/VDDIO to stabilize.. done */
    /* step 2 */
    delay_us(TIMER0, 50000);

    /* step 3 */
    P0->OUTCLR = GPIO0;
    delay_us(TIMER0, 1000);
    P0->OUTSET = GPIO0;
    delay_us(TIMER0, 1000);
    P0->OUTCLR = GPIO0;

    /* step 4 */
    paw_write(PAW3395_POWER_UP_RESET, 0x5A);

    /* step 5 */
    delay_us(TIMER0, 10000);

    /* step 6 */
    paw_write(0x7F, 0x07);  /* 1 */
    paw_write(0x40, 0x41);  /* 2 */
    paw_write(0x7F, 0x00);  /* 3 */
    paw_write(0x40, 0x80);  /* 4 */
    paw_write(0x7F, 0x0E);  /* 5 */
    paw_write(0x55, 0x0D);  /* 6 */
    paw_write(0x56, 0x1B);  /* 7 */
    paw_write(0x57, 0xE8);  /* 8 */
    paw_write(0x58, 0xD5);  /* 9 */
    paw_write(0x7F, 0x14);  /* 10 */
    paw_write(0x42, 0xBC);  /* 11 */
    paw_write(0x43, 0x74);  /* 12 */
    paw_write(0x4B, 0x20);  /* 13 */
    paw_write(0x4D, 0x00);  /* 14 */
    paw_write(0x53, 0x0E);  /* 15 */
    paw_write(0x7F, 0x05);  /* 16 */
    paw_write(0x44, 0x04);  /* 17 */
    paw_write(0x4D, 0x06);  /* 18 */
    paw_write(0x51, 0x40);  /* 19 */
    paw_write(0x53, 0x40);  /* 20 */
    paw_write(0x55, 0xCA);  /* 21 */
    paw_write(0x5A, 0xE8);  /* 22 */
    paw_write(0x5B, 0xEA);  /* 23 */
    paw_write(0x61, 0x31);  /* 24 */
    paw_write(0x62, 0x64);  /* 25 */
    paw_write(0x6D, 0xB8);  /* 26 */
    paw_write(0x6E, 0x0F);  /* 27 */
    paw_write(0x70, 0x02);  /* 28 */
    paw_write(0x4A, 0x2A);  /* 29 */
    paw_write(0x60, 0x26);  /* 30 */
    paw_write(0x7F, 0x06);  /* 31 */
    paw_write(0x6D, 0x70);  /* 32 */
    paw_write(0x6E, 0x60);  /* 33 */
    paw_write(0x6F, 0x04);  /* 34 */
    paw_write(0x53, 0x02);  /* 35 */
    paw_write(0x55, 0x11);  /* 36 */
    paw_write(0x7A, 0x01);  /* 37 */
    paw_write(0x7D, 0x51);  /* 38 */
    paw_write(0x7F, 0x07);  /* 39 */
    paw_write(0x41, 0x10);  /* 40 */
    paw_write(0x42, 0x32);  /* 41 */
    paw_write(0x43, 0x00);  /* 42 */
    paw_write(0x7F, 0x08);  /* 43 */
    paw_write(0x71, 0x4F);  /* 44 */
    paw_write(0x7F, 0x09);  /* 45 */
    paw_write(0x62, 0x1F);  /* 46 */
    paw_write(0x63, 0x1F);  /* 47 */
    paw_write(0x65, 0x03);  /* 48 */
    paw_write(0x66, 0x03);  /* 49 */
    paw_write(0x67, 0x1F);  /* 50 */
    paw_write(0x68, 0x1F);  /* 51 */
    paw_write(0x69, 0x03);  /* 52 */
    paw_write(0x6A, 0x03);  /* 53 */
    paw_write(0x6C, 0x1F);  /* 54 */
    paw_write(0x6D, 0x1F);  /* 55 */
    paw_write(0x51, 0x04);  /* 56 */
    paw_write(0x53, 0x20);  /* 57 */
    paw_write(0x54, 0x20);  /* 58 */
    paw_write(0x71, 0x0C);  /* 59 */
    paw_write(0x72, 0x07);  /* 60 */
    paw_write(0x73, 0x07);  /* 61 */
    paw_write(0x7F, 0x0A);  /* 62 */
    paw_write(0x4A, 0x14);  /* 63 */
    paw_write(0x4C, 0x14);  /* 64 */
    paw_write(0x55, 0x19);  /* 65 */
    paw_write(0x7F, 0x14);  /* 66 */
    paw_write(0x4B, 0x30);  /* 67 */
    paw_write(0x4C, 0x03);  /* 68 */
    paw_write(0x61, 0x0B);  /* 69 */
    paw_write(0x62, 0x0A);  /* 70 */
    paw_write(0x63, 0x02);  /* 71 */
    paw_write(0x7F, 0x15);  /* 72 */
    paw_write(0x4C, 0x02);  /* 73 */
    paw_write(0x56, 0x02);  /* 74 */
    paw_write(0x41, 0x91);  /* 75 */
    paw_write(0x4D, 0x0A);  /* 76 */
    paw_write(0x7F, 0x0C);  /* 77 */
    paw_write(0x4A, 0x10);  /* 78 */
    paw_write(0x4B, 0x0C);  /* 79 */
    paw_write(0x4C, 0x40);  /* 80 */
    paw_write(0x41, 0x25);  /* 81 */
    paw_write(0x55, 0x18);  /* 82 */
    paw_write(0x56, 0x14);  /* 83 */
    paw_write(0x49, 0x0A);  /* 84 */
    paw_write(0x42, 0x00);  /* 85 */
    paw_write(0x43, 0x2D);  /* 86 */
    paw_write(0x44, 0x0C);  /* 87 */
    paw_write(0x54, 0x1A);  /* 88 */
    paw_write(0x5A, 0x0D);  /* 89 */
    paw_write(0x5F, 0x1E);  /* 90 */
    paw_write(0x5B, 0x05);  /* 91 */
    paw_write(0x5E, 0x0F);  /* 92 */
    paw_write(0x7F, 0x0D);  /* 93 */
    paw_write(0x48, 0xDD);  /* 94 */
    paw_write(0x4F, 0x03);  /* 95 */
    paw_write(0x52, 0x49);  /* 96 */
    paw_write(0x51, 0x00);  /* 97 */
    paw_write(0x54, 0x5B);  /* 98 */
    paw_write(0x53, 0x00);  /* 99 */
    paw_write(0x56, 0x64);  /* 100 */
    paw_write(0x55, 0x00);  /* 101 */
    paw_write(0x58, 0xA5);  /* 102 */
    paw_write(0x57, 0x02);  /* 103 */
    paw_write(0x5A, 0x29);  /* 104 */
    paw_write(0x5B, 0x47);  /* 105 */
    paw_write(0x5C, 0x81);  /* 106 */
    paw_write(0x5D, 0x40);  /* 107 */
    paw_write(0x71, 0xDC);  /* 108 */
    paw_write(0x70, 0x07);  /* 109 */
    paw_write(0x73, 0x00);  /* 110 */
    paw_write(0x72, 0x08);  /* 111 */
    paw_write(0x75, 0xDC);  /* 112 */
    paw_write(0x74, 0x07);  /* 113 */
    paw_write(0x77, 0x00);  /* 114 */
    paw_write(0x76, 0x08);  /* 115 */
    paw_write(0x7F, 0x10);  /* 116 */
    paw_write(0x4C, 0xD0);  /* 117 */
    paw_write(0x7F, 0x00);  /* 118 */
    paw_write(0x4F, 0x63);  /* 119 */
    paw_write(0x4E, 0x00);  /* 120 */
    paw_write(0x52, 0x63);  /* 121 */
    paw_write(0x51, 0x00);  /* 122 */
    paw_write(0x54, 0x54);  /* 123 */
    paw_write(0x5A, 0x10);  /* 124 */
    paw_write(0x77, 0x4F);  /* 125 */
    paw_write(0x47, 0x01);  /* 126 */
    paw_write(0x5B, 0x40);  /* 127 */
    paw_write(0x64, 0x60);  /* 128 */
    paw_write(0x65, 0x06);  /* 129 */
    paw_write(0x66, 0x13);  /* 130 */
    paw_write(0x67, 0x0F);  /* 131 */
    paw_write(0x78, 0x01);  /* 132 */
    paw_write(0x79, 0x9C);  /* 133 */
    paw_write(0x40, 0x00);  /* 134 */
    paw_write(0x55, 0x02);  /* 135 */
    paw_write(0x23, 0x70);  /* 136 */
    paw_write(0x22, 0x01);  /* 137 */

    delay_us(TIMER0, 1000);         /* 138 */

    /* 139 */
    uint8_t val;
    int i;
    for (i = 0; i < 60; i++) {
        val = paw_read(0x6C);
        if (val == 0x80) {
            break;
        }
        delay_us(TIMER0, 1000);
    }
    if (i == 60) {
        paw_write(0x7F, 0x14);  /* a */
        paw_write(0x6C, 0x00);  /* b */
        paw_write(0x7F, 0x00);  /* c */
    }

    paw_write(0x22, 0x00);  /* 140 */
    paw_write(0x55, 0x00);  /* 141 */
    paw_write(0x7F, 0x07);  /* 142 */
    paw_write(0x40, 0x40);  /* 143 */
    paw_write(0x7F, 0x00);  /* 144 */
    paw_write(0x68, 0x01);  /* 145 */

    /* step 7 */
    (void)paw_read(0x02);
    (void)paw_read(0x03);
    (void)paw_read(0x04);
    (void)paw_read(0x05);
    (void)paw_read(0x06);

    /* run->rest1 downshift = 15s, rest1->rest2 = 30s, rest2->rest3 = 64s */
    paw_modify(PAW3395_RUN_DOWNSHIFT_MULT, PAW3395_RUN_DOWNSHIFT_MULT_RUN_M_Msk, 
               PAW3395_RUN_DOWNSHIFT_MULT_RUN_M_2048);
    paw_write(PAW3395_RUN_DOWNSHIFT, 146);
    paw_write(PAW3395_REST1_DOWNSHIFT, 234);
    paw_write(PAW3395_REST2_DOWNSHIFT, 10);

    /* clear bit that causes inversion of X axis */
    paw_modify(PAW3395_AXIS_CONTROL, PAW3395_AXIS_CONTROL_INVX_, 0);

}

/* DPI must be a multiple of 50
 * min: 50, max: 26000
 */
void paw_set_dpi(uint16_t dpi) {

    uint16_t res     = dpi / 50;
    uint8_t res_low  = (res >> 0) & 0x00FF;
    uint8_t res_high = (res >> 8) & 0x00FF;

    /* set dpi */
    paw_write(PAW3395_RES_X_LOW,  res_low);
    paw_write(PAW3395_RES_X_HIGH, res_high);
    paw_write(PAW3395_RES_Y_LOW,  res_low);
    paw_write(PAW3395_RES_Y_HIGH, res_high);
    paw_modify(PAW3395_SET_RESOLUTION, 0, PAW3395_SET_RESOLUTION_SET_RES_);

    /* datasheet recommends for DPI >= 9000 */
    if (dpi >= 9000) {
        paw_modify(PAW3395_RIPPLE_CONTROL, 0, PAW3395_RIPPLE_CONTROL_CTRL8_);
    }

}

