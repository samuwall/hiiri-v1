/**********************************************************************************
 ** file            : paw3395.h
 ** description     :
 **
 **********************************************************************************/

#ifndef PAW3395_H
#define PAW3395_H

#include <stdint.h>

#define BURST_SIZE 6

/* --- REGISTER ADDRESSES ------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */

#define PAW3395_PRODUCT_ID          0x00
#define PAW3395_MOTION_BURST        0x16
#define PAW3395_POWER_UP_RESET      0x3A
#define PAW3395_PERFORMANCE         0x40
#define PAW3395_SET_RESOLUTION      0x47
#define PAW3395_RES_X_LOW           0x48
#define PAW3395_RES_X_HIGH          0x49
#define PAW3395_RES_Y_LOW           0x4A
#define PAW3395_RES_Y_HIGH          0x4B
#define PAW3395_RIPPLE_CONTROL      0x5A
#define PAW3395_AXIS_CONTROL        0x5B
#define PAW3395_RUN_DOWNSHIFT       0x77
#define PAW3395_REST1_DOWNSHIFT     0x79
#define PAW3395_REST2_DOWNSHIFT     0x7B
#define PAW3395_RUN_DOWNSHIFT_MULT  0x7D

/* --- BITFIELDS --------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

#define PAW3395_MOTION_OP_MODE_Msk            (0b11 << 0)
#define PAW3395_MOTION_OP_MODE_Run            (0b00 << 0)
#define PAW3395_MOTION_OP_MODE_Rest1          (0b01 << 0)
#define PAW3395_MOTION_OP_MODE_Rest2          (0b10 << 0)
#define PAW3395_MOTION_OP_MODE_Rest3          (0b11 << 0)
#define PAW3395_PERFORMANCE_AWAKE_            (1 << 7)
#define PAW3395_SET_RESOLUTION_SET_RES_       (1 << 0)
#define PAW3395_RIPPLE_CONTROL_CTRL8_         (1 << 7)
#define PAW3395_AXIS_CONTROL_INVX_            (1 << 5)
#define PAW3395_RUN_DOWNSHIFT_MULT_RUN_M_Msk  (0b1111 << 0)
#define PAW3395_RUN_DOWNSHIFT_MULT_RUN_M_2048 (0b1010 << 0)

/* --- FUNCTION DECLARATIONS --------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

void paw_init(void);
void paw_set_dpi(uint16_t dpi);

#endif
