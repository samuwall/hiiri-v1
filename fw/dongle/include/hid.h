/**********************************************************************************
 ** file            : hid.h
 ** description     : 
 ** 
 **********************************************************************************/

#ifndef HID_H
#define HID_H

#include <stdint.h>

/* ----------------------------------------------------------------------------------- */
/* --- USB HID ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* HID1_11: 4.1 */
#define USB_CLASS_HID                           3

/* HID1_11: 4.2 */
#define USB_HID_SUBCLASS_NO                     0
#define USB_HID_SUBCLASS_BOOT_INTERFACE         1

/* HID1_11: 4.3 */
#define USB_HID_INTERFACE_PROTOCOL_NONE         0
#define USB_HID_INTERFACE_PROTOCOL_KEYBOARD     1
#define USB_HID_INTERFACE_PROTOCOL_MOUSE        2

/* HID1_11: 7.1 */
#define USB_HID_DT_HID                          0x21
#define USB_HID_DT_REPORT                       0x22
#define USB_HID_DT_PHYSICAL                     0x23

/* HID1_11: 7.2 */
#define USB_HID_REQ_TYPE_GET_REPORT             0x01
#define USB_HID_REQ_TYPE_GET_IDLE               0x02
#define USB_HID_REQ_TYPE_GET_PROTOCOL           0x03
#define USB_HID_REQ_TYPE_SET_REPORT             0x09
#define USB_HID_REQ_TYPE_SET_IDLE               0x0A
#define USB_HID_REQ_TYPE_SET_PROTOCOL           0x0B

/* HID1_11: 7.2.1 */
#define USB_HID_REPORT_TYPE_INPUT               1
#define USB_HID_REPORT_TYPE_OUTPUT              2
#define USB_HID_REPORT_TYPE_FEATURE             3

/* HID1_11: 7.2.5 */
#define USB_HID_PROTOCOL_BOOT                   0
#define USB_HID_PROTOCOL_REPORT                 1

/* HID1_11: 6.2.1 (note 1) */
struct usb_hid_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdHID;
    uint8_t  bCountryCode;
    uint8_t  bNumDescriptors;
    uint8_t  bReportDescriptorType;
    uint16_t wReportDescriptorLength;
} __attribute__((packed));

/* note 1 :  hid_descriptor.. 
*         
*           HID1_11 6.2.1 specifies that the `HID Descriptor` as a whole contains
*           the HID descriptor followed by some number of "optional" descriptors,
*           which are either report or physical descriptors. physical descriptors
*           sound kind useless, and 1 report descriptor is all you need given 
*           that one report descriptor can define multiple reports using the 
*           Report ID collection. 
* 
*           lets just define one hid desc struct. users can easily add optional 
*           desciptors if necessary
*/

#endif