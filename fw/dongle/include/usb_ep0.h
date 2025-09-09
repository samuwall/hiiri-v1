/***********************************************************************************
 ** file            : usb_ep0.h
 ** description     : these functions are exported to usb.c, where they are used
 **                   in usb_init() to be set as CTR callbacks for endpoint 0
 ** 
 **********************************************************************************/

#ifndef USB_EP0_H
#define USB_EP0_H

#include "usb.h"

void usb_ep0_setup(usb_device *dev, uint8_t ep);
void usb_ep0_out(usb_device *dev, uint8_t ep);
void usb_ep0_in(usb_device *dev, uint8_t ep);

#endif