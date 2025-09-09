/**********************************************************************************
 ** file            : paw3395.h
 ** description     : helper functions for paw3395 (nRF52)
 **
 **********************************************************************************/

#ifndef PAW3395_H
#define PAW3395_H

#include <stdint.h>

#define BURST_SIZE 6

/* --- REGISTER ADDRESSES ------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */

#define PAW3395_PRODUCT_ID          0x00    /* R   */
#define PAW3395_REVISION_ID         0x01    /* R   */
#define PAW3395_MOTION              0x02    /* R/W */
#define PAW3395_DELTA_X_L           0x03    /* R   */
#define PAW3395_DELTA_X_H           0x04    /* R   */
#define PAW3395_DELTA_Y_L           0x05    /* R   */
#define PAW3395_DELTA_Y_H           0x06    /* R   */
#define PAW3395_SQUAL               0x07    /* R   */
#define PAW3395_RAWDATA_SUM         0x08    /* R   */
#define PAW3395_MAXIMUM_RAWDATA     0x09    /* R   */
#define PAW3395_MINIMUM_RAWDATA     0x0A    /* R   */
#define PAW3395_SHUTTER_LOWER       0x0B    /* R   */
#define PAW3395_SHUTTER_UPPER       0x0C    /* R   */
#define PAW3395_OBSERVATION         0x15    /* R/W */
#define PAW3395_MOTION_BURST        0x16    /* R/W */
#define PAW3395_POWER_UP_RESET      0x3A    /* W   */
#define PAW3395_SHUTDOWN            0x3B    /* W   */
#define PAW3395_PERFORMANCE         0x40    /* R   */
#define PAW3395_SET_RESOLUTION      0x47    /* W   */
#define PAW3395_RES_X_LOW           0x48    /* R/W */
#define PAW3395_RES_X_HIGH          0x49    /* R/W */
#define PAW3395_RES_Y_LOW           0x4A    /* R/W */
#define PAW3395_RES_Y_HIGH          0x4B    /* R/W */
#define PAW3395_ANGLE_SNAP          0x56    /* R/W */
#define PAW3395_RAWDATA_OUTPUT      0x58    /* R   */
#define PAW3395_RAWDATA_STATUS      0x59    /* R   */
#define PAW3395_RIPPLE_CONTROL      0x5A    /* R/W */
#define PAW3395_AXIS_CONTROL        0x5B    /* R/W */
#define PAW3395_MOTION_CTRL         0x5C    /* R/W */
#define PAW3395_INV_PRODUCT_ID      0x5F    /* R   */
#define PAW3395_RUN_DOWNSHIFT       0x77    /* R/W */
#define PAW3395_REST1_PERIOD        0x78    /* R/W */
#define PAW3395_REST1_DOWNSHIFT     0x79    /* R/W */
#define PAW3395_REST2_PERIOD        0x7A    /* R/W */
#define PAW3395_REST2_DOWNSHIFT     0x7B    /* R/W */
#define PAW3395_REST3_PERIOD        0x7C    /* R/W */
#define PAW3395_RUN_DOWNSHIFT_MULT  0x7D    /* R/W */
#define PAW3395_REST_DOWNSHIFT_MULT 0x7E    /* R/W */

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
