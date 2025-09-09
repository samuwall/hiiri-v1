/**********************************************************************************
 ** file            : usb_ep0.c
 ** description     : these functions handle all endpoint 0 (control pipe)
 **                   logic. this includes functions which handle standard 
 **                   device/interface/endpoint requests.
 **
 **                   <https://www.beyondlogic.org/usbnutshell/usb4.shtml>
 **                   <https://www.beyondlogic.org/usbnutshell/usb6.shtml>
 **
 **                   separates the low-level driver / hw interface [usb.c] 
 **                   from the high-level control transfer logic [usb_ep0.c]. 
 **                   as such, it can be re-used with different low-level 
 **                   USB drivers (theoretically).
 ** 
 **********************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "device.h"
#include "utils.h"
#include "usb.h"
#include "usb_ep0.h"

#include "SEGGER_RTT.h"

/* ----------------------------------------------------------------------------------- */
/* --- USB STANDARD REQUEST HANDLERS (device, interface, endpoint) ------------------- */
/* ----------------------------------------------------------------------------------- */

static enum usb_req_result
usb_std_req_device_get_status(usb_device *dev, struct usb_setup_data *req, 
                               uint8_t **buf, uint16_t *len) {
    (void)req;

    *len = 2;
    *buf = (uint8_t *) &dev->status;

    return USB_REQ_HANDLED;
}

static enum usb_req_result
usb_std_req_device_set_address(usb_device *dev, struct usb_setup_data *req, 
                                uint8_t **buf, uint16_t *len) {
    (void)dev;
    (void)req;
    (void)buf;
    (void)len;

    /* handled at STATUS_IN */
    return USB_REQ_HANDLED;
}

static enum usb_req_result
usb_std_req_device_get_descriptor(usb_device *dev, struct usb_setup_data *req, 
                                   uint8_t **buf, uint16_t *len) {

    /* 9.4.3: descriptor index in the low byte, 
     * descriptor type in the high byte 
     */
    uint8_t desc_id   = req->wValue & 0xFF;
    uint8_t desc_type = req->wValue >> 8;
    
    switch (desc_type) {

        case USB_DT_DEVICE:

            /* handle non-specified host behavior, see note 3 */
            if ((*len == 64) && (!dev->configured)) {
                *len = dev->dev_desc->bMaxPacketSize0;
                dev->ep0.req.wLength = *len;
            }

            *buf = (uint8_t *) dev->dev_desc;
            *len = MIN(*len, dev->dev_desc->bLength);

            return USB_REQ_HANDLED;

        case USB_DT_CONFIGURATION:

            /* beyond the top-level configuration descriptor should
             * lie a contiguous block of memory, representing the
             * entire configuration descriptor block, in the order
             * described by USB_20 9.4.3
             */
            *buf = (uint8_t *) dev->config;
            *len = MIN(*len, dev->config->wTotalLength);
            
            return USB_REQ_HANDLED;

        case USB_DT_STRING:

            if (desc_id >= dev->num_str_descs) {
                /* req'd str out of bounds: stall */
                return USB_REQ_ERR;
            }

            *buf = (uint8_t *) dev->str_descs[desc_id];
            *len = MIN(*len, dev->str_descs[desc_id]->bLength);

            return USB_REQ_HANDLED;

        default:
            return USB_REQ_ERR;
    }
    
}

static enum usb_req_result
usb_std_req_device_set_configuration(usb_device *dev, struct usb_setup_data *req, 
                                      uint8_t **buf, uint16_t *len) {

    (void)buf;
    (void)len;
    
    if (req->wValue == 0) {
        /* enter/remain in non-configured address state */
        usb_reset_endpoints(dev);
        dev->configured = 0;
        return USB_REQ_HANDLED;
    }
    
    if (req->wValue != dev->config->bConfigurationValue) {
        /* host requested invalid config */
        return USB_REQ_ERR;
    }
    
    /* config found: enter configured state */
    usb_reset_endpoints(dev);

    /* run user set_config callback */
    if (dev->user_set_config_callback) {
        dev->user_set_config_callback (dev, req->wValue);
    }

    dev->configured = 1;
    return USB_REQ_HANDLED;
}

/* --- STANDARD REQUEST HANDLER ------------------------------------------------------ */

static enum usb_req_result
usb_std_req_handler(usb_device *dev, struct usb_setup_data *req, 
                         uint8_t **buf, uint16_t *len) {
    
    if ((req->bmRequestType & USB_REQ_TYPE_TYPE) != USB_REQ_TYPE_STANDARD) {
        return USB_REQ_ERR;
    }

    switch (req->bmRequestType & USB_REQ_TYPE_RECIPIENT) {

    case USB_REQ_TYPE_DEVICE:

        switch (req->bRequest) {
            case USB_REQ_GET_STATUS:
                return usb_std_req_device_get_status(dev, req, buf, len);

            case USB_REQ_CLEAR_FEATURE:
            case USB_REQ_SET_FEATURE:
                /* only used for remote wakeup/test mode - not supported here */
                return USB_REQ_ERR;

            case USB_REQ_SET_ADDRESS:
                return usb_std_req_device_set_address(dev, req, buf, len);

            case USB_REQ_GET_DESCRIPTOR:
                return usb_std_req_device_get_descriptor(dev, req, buf, len);

            case USB_REQ_SET_DESCRIPTOR:
                /* optional per USB spec */
                return USB_REQ_ERR;

            case USB_REQ_GET_CONFIGURATION:
                /* unlikely request: not yet implemented */
                return USB_REQ_ERR;

            case USB_REQ_SET_CONFIGURATION:
                return usb_std_req_device_set_configuration(dev, req, buf, len);

            default:
                return USB_REQ_ERR;
        }

    case USB_REQ_TYPE_INTERFACE:

        switch (req->bRequest) {
            case USB_REQ_GET_STATUS:
            case USB_REQ_CLEAR_FEATURE:
            case USB_REQ_SET_FEATURE:
            case USB_REQ_GET_INTERFACE:
            case USB_REQ_SET_INTERFACE:
            default:
                /* not yet implemented */
                return USB_REQ_ERR;
        }

    case USB_REQ_TYPE_ENDPOINT:

        switch (req->bRequest) {
            case USB_REQ_GET_STATUS:
            case USB_REQ_CLEAR_FEATURE:
            case USB_REQ_SET_FEATURE:
            case USB_REQ_SET_SYNCH_FRAME:
            default:
                /* not yet implemented */
                return USB_REQ_ERR;
        }

    default:
        return USB_REQ_ERR;
    }

}

/* ----------------------------------------------------------------------------------- */
/* --- USB ENDPOINT 0 (CONTROL TRANSFER) LOGIC --------------------------------------- */
/* ----------------------------------------------------------------------------------- */

static enum usb_req_result
usb_ep0_handle_request(usb_device *dev, struct usb_setup_data *req) {
    
    int result = 0;
    struct user_ep0_req_handler *handler = dev->user_ep0_req_handler;

    for (int i = 0; i < MAX_USER_EP0_REQ_HANDLER; i++) {

        if (handler[i].cb == NULL) {
            break;
        }

        if ((req->bmRequestType & handler[i].type_mask) == handler[i].type) {
            result = handler[i].cb(dev, req, &(dev->ep0.xfer_buf), 
                                             &(dev->ep0.xfer_len),
                                             &(dev->ep0.req_cmpl));
            if (result == USB_REQ_HANDLED || result == USB_REQ_ERR) {
                return result;
            }
        }
        
    }

    /* if no user routine for request is found, use standard request handlers */
    result = usb_std_req_handler(dev, req, &(dev->ep0.xfer_buf),
                                           &(dev->ep0.xfer_len));

    return result;
}

static void usb_ep0_data_in(usb_device *dev) {

    if (dev->dev_desc->bMaxPacketSize0 < dev->ep0.xfer_len) {

        /* data stage: more data to send at next IN packet */
        usb_ep_write_packet(dev, 0, dev->ep0.xfer_buf, dev->dev_desc->bMaxPacketSize0);

        dev->ep0.xfer_buf += dev->dev_desc->bMaxPacketSize0;
        dev->ep0.xfer_len -= dev->dev_desc->bMaxPacketSize0;
        dev->ep0.stage = USB_DATA_IN;

    }
    else {

        /* data stage: all data sent with this packet */
        usb_ep_write_packet(dev, 0, dev->ep0.xfer_buf, dev->ep0.xfer_len);
        
        if (dev->ep0.short_xfer && (dev->ep0.xfer_len == dev->dev_desc->bMaxPacketSize0)) {
            /* send ZLP */
            dev->ep0.xfer_len   = 0;
            dev->ep0.xfer_buf   = NULL;
            dev->ep0.stage      = USB_DATA_IN;
        }
        else {
            dev->ep0.stage = USB_LAST_DATA_IN;
        }
        
    }

}

void usb_ep0_setup(usb_device *dev, uint8_t ep) {

    (void)ep;

    usb_setup_acked(dev);
    struct usb_setup_data *req = &(dev->ep0.req);

    #if DBG >= 1
    SEGGER_RTT_printf(0, "bmRequestType: x%02X  bRequest: %03d  wValue: x%04X  wIndex: x%04X  wLength: x%04X\n", 
                      req->bmRequestType, req->bRequest, req->wValue, req->wIndex, req->wLength);
    #endif

    if ((req->wLength == 0) || (req->bmRequestType & USB_REQ_TYPE_IN)) {

        dev->ep0.xfer_buf = NULL;
        dev->ep0.xfer_len = req->wLength;

        if (usb_ep0_handle_request(dev, req) == USB_REQ_HANDLED) {

            if (req->wLength > 0) {
                /* see note 1 */
                dev->ep0.short_xfer = dev->ep0.xfer_len &&
                                     (dev->ep0.xfer_len < req->wLength);
                /* send first data packet */
                usb_ep0_data_in(dev);
            }
            else {
                /* no data stages, go to status stage */
                usb_prepare_for_status(dev, USB_STATUS_IN);
                dev->ep0.stage = USB_STATUS_IN;
            }

        }
        else {
            /* request error: stall endpoint */
            usb_ep_set_stall(dev, 0);
            dev->ep0.stage = USB_IDLE;
        }
    }
    else {

        /* host wants to send data stages to us over ep0. there's rly no
         * good reason for this to happen so we just stall.
         *
         * the only std req that requires this is SET_DESCRIPTOR, which
         * is officially optional and would never happen. otherwise, we'll
         * just assume any vendor-specific data that you wanna send from the
         * host over ep0 can fit in the setup data (req->wIndex, req->wValue)
         */
        usb_ep_set_stall(dev, 0);
        dev->ep0.stage = USB_IDLE;
    }

}

void usb_ep0_in(usb_device *dev, uint8_t ep) {

    (void)ep;

    switch (dev->ep0.stage) {

        case USB_DATA_IN:

            #if DBG >= 1
            SEGGER_RTT_printf(0, "    DATA_IN\n");
            #endif

            usb_ep0_data_in(dev);
            break;
        
        case USB_LAST_DATA_IN:

            #if DBG >= 1
            SEGGER_RTT_printf(0, "    LAST_DATA_IN\n");
            #endif

            usb_prepare_for_status(dev, USB_STATUS_OUT);
            dev->ep0.stage = USB_STATUS_OUT;
            break;

        case USB_STATUS_IN:

            #if DBG >= 1
            SEGGER_RTT_printf(0, "    STATUS_IN\n");
            #endif

            usb_status_acked(dev, USB_STATUS_IN);
            dev->ep0.stage = USB_IDLE;
            break;
        
        default:
            usb_ep_set_stall(dev, 0);
            dev->ep0.stage = USB_IDLE;

    }

}

void usb_ep0_out(usb_device *dev, uint8_t ep) {
    
    (void)ep;

    switch (dev->ep0.stage) {

        case USB_STATUS_OUT:

            #if DBG >= 1
            SEGGER_RTT_printf(0, "    STATUS_OUT\n");
            #endif

            usb_status_acked(dev, USB_STATUS_OUT);
            dev->ep0.stage = USB_IDLE;
            break;

        default:
            usb_ep_set_stall(dev, 0);
            dev->ep0.stage = USB_IDLE;
            
    }

}

/* ----------------------------------------------------------------------------------- */
/* --- USER API ---------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* user API for registering a 'set config' callback. e.g., `hid_set_config()` 
 */
void usb_register_set_config_callback(usb_device *dev, usb_set_config_callback callback) {
    dev->user_set_config_callback = callback;
}

/* user API for registering control request handlers, e.g., `handle_get_hid_report_descriptor()`
 */
int usb_register_ep0_req_handler(usb_device *dev, uint8_t type, 
                                 uint8_t type_mask, usb_ep0_req_handler callback) {
                                    
    for (int i = 0; i < MAX_USER_EP0_REQ_HANDLER; i++) {

        if (!dev->user_ep0_req_handler[i].cb) {
            dev->user_ep0_req_handler[i].type = type;
            dev->user_ep0_req_handler[i].type_mask = type_mask;
            dev->user_ep0_req_handler[i].cb = callback;
            return 0;
        }
    }

    return -1;
}

/* ----------------------------------------------------------------------------------- */
/* --- NOTES ------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* note 1 : dev->short_xfer...
 *         
 *          regarding the need for a zero-length packet with short transfers..
 * 
 *          Section 8.5.3.2 Variable-length Data Stage: 
 *              A control pipe may have a variable-length data phase in which the 
 *              host requests more data than is contained in the specified data 
 *              structure. When all of the data structure is returned to the host, 
 *              the function should indicate that the Data stage is ended by returning
 *              a packet that is shorter than the MaxPacketSize for the pipe. If the 
 *              data structure is an exact multiple of wMaxPacketSize for the pipe, 
 *              the function will return a zero-length packet to indicate the end of 
 *              the Data stage.
 * 
 *          normally, the USB host will know that the transaction is over either when it
 *          receives all of the data it requested in wLength, or for short transfers, 
 *          when it sees a packet with a length less than wMaxPacketSize (our ep size), 
 *          indicating logically that those were the last bytes. however, if the total data
 *          happens to be a multiple of wMaxPacketSize (1x, 2x, 3x, etc..), then the host 
 *          has no way of knowing if we still have more data to send, and it may wait 
 *          indefinitely. the ZLP is how we tell the host we have no more data to send. 
 *          
 *       
 * note 2 :  usb_std_req_set_address()  ...  usb_ep0_in() 
 * 
 *          the reason why `usb_std_req_set_address()` doesn't actually set the 
 *          device address during the request handling stage and why it is instead 
 *          eventually set at the STATUS IN stage is that the host expects the 
 *          device/function to respond to the same device address throughout the 
 *          entire control transfer. The STATUS IN stage represents the end of the
 *          control transfer. ( see section 9.4.6 )
 *    
 * note 3 : get_device_descriptor()
 * 
 *          handle non-specified host behavior that breaks enumeration when
 *          bMaxPacketSize0 <= 16:
 * 
 *          linux/windows host will first probe with a 64 byte req for dev desc
 *          before even knowing our maxPacketSize. it will then for some reason send
 *          a STATUS OUT after the first data stage, even if our bMaxPacketSize0
 *          dictates that sending all 18 bytes of the desc should take multiple
 *          data stages. so, we force the requested len to be our bmaxPacketSize0, 
 *          such that we send the data in one stage and are ready for the STATUS OUT.
 * 
 */
