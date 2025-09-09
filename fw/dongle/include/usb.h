/***********************************************************************************
 ** file            : usb.h
 ** description     : 
 ** 
 **********************************************************************************/

#ifndef USB_H
#define USB_H

#include <stdint.h>
#include "device.h"

/* ----------------------------------------------------------------------------------- */
/* --- USB_20 -- 9.3/9.4: STANDARD DEVICE REQUESTS ----------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* table 9-2: setup data format */
struct usb_setup_data {
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed, aligned(4)));

#define USB_SETUP_DATA_SIZE sizeof(struct usb_setup_data)

/* table 9-2: bmRequestType bitfields */

/* [7] direction */
#define USB_REQ_TYPE_DIRECTION                  0b10000000
#define USB_REQ_TYPE_IN                         0b10000000
#define USB_REQ_TYPE_OUT                        0b00000000
/* [6:5] type */
#define USB_REQ_TYPE_TYPE                       0b01100000
#define USB_REQ_TYPE_STANDARD                   0b00000000
#define USB_REQ_TYPE_CLASS                      0b00100000
#define USB_REQ_TYPE_VENDOR                     0b01000000
/* [0:4] recipient */
#define USB_REQ_TYPE_RECIPIENT                  0b00011111
#define USB_REQ_TYPE_DEVICE                     0b00000000
#define USB_REQ_TYPE_INTERFACE                  0b00000001
#define USB_REQ_TYPE_ENDPOINT                   0b00000010
#define USB_REQ_TYPE_OTHER                      0b00000011

/* table 9-4: bRequest codes */

#define USB_REQ_GET_STATUS                      0
#define USB_REQ_CLEAR_FEATURE                   1
#define USB_REQ_SET_FEATURE                     3
#define USB_REQ_SET_ADDRESS                     5
#define USB_REQ_GET_DESCRIPTOR                  6
#define USB_REQ_SET_DESCRIPTOR                  7
#define USB_REQ_GET_CONFIGURATION               8
#define USB_REQ_SET_CONFIGURATION               9
#define USB_REQ_GET_INTERFACE                   10
#define USB_REQ_SET_INTERFACE                   11
#define USB_REQ_SET_SYNCH_FRAME                 12

/* table 9-5: descriptor type codes */

#define USB_DT_DEVICE                           1
#define USB_DT_CONFIGURATION                    2
#define USB_DT_STRING                           3
#define USB_DT_INTERFACE                        4
#define USB_DT_ENDPOINT                         5
#define USB_DT_DEVICE_QUALIFIER                 6
#define USB_DT_OTHER_SPEED_CONFIGURATION        7
#define USB_DT_INTERFACE_POWER                  8

/* ----------------------------------------------------------------------------------- */
/* --- USB_20 -- 9.5/9.6: STANDARD USB DESCRIPTOR DEFINITIONS ------------------------ */
/* ----------------------------------------------------------------------------------- */

/* table 9-8: standard device descriptor */
struct usb_device_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} __attribute__((packed));

#define USB_DT_DEVICE_SIZE sizeof(struct usb_device_descriptor)

/* table 9-10: standard configuration descriptor */
struct usb_configuration_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} __attribute__((packed));

#define USB_DT_CONFIGURATION_SIZE sizeof(struct usb_configuration_descriptor)

/* table 9-10: configuration bmAttributes bitfields */

/* [7] reserved (set to one) */
#define USB_CFG_ATTR_RESERVED                   0b10000000
/* [6] self-powered */
#define USB_CFG_ATTR_SELF_POWERED               0b01000000
/* [5] remote wakeup */
#define USB_CFG_ATTR_REMOTE_WAKEUP              0b00100000

/* figure 9-4: GetStatus() return data */

#define USB_STATUS_SELF_POWERED                 0x0
#define USB_STATUS_BUS_POWERED                  0x1

/* table 9-12: standard interface descriptor */
struct usb_interface_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed));

#define USB_DT_INTERFACE_SIZE sizeof(struct usb_interface_descriptor)

/* table 9-13: standard endpoint descriptor */
struct usb_endpoint_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;  
} __attribute__((packed));

#define USB_DT_ENDPOINT_SIZE sizeof(struct usb_endpoint_descriptor)

/* table 9-13: endpoint bmAttributes bitfields */

/* [5:4] usage type (ISO ONLY, 0x00 otherwise) */
#define USB_EP_ATTR_USAGETYPE                   0b00110000
#define USB_EP_ATTR_DATA                        0b00000000
#define USB_EP_ATTR_FEEDBACK                    0b00010000
#define USB_EP_ATTR_IMPLICIT_FEEDBACK_DATA      0b00100000
/* [3:2] sync type (ISO ONLY, 0x00 otherwise) */
#define USB_EP_ATTR_SYNCTYPE                    0b00001100
#define USB_EP_ATTR_NOSYNC                      0b00000000
#define USB_EP_ATTR_ASYNC                       0b00000100
#define USB_EP_ATTR_ADAPTIVE                    0b00001000
#define USB_EP_ATTR_SYNC                        0b00001100
/* [1:0] transfer type */
#define USB_EP_ATTR_TYPE                        0b00000011
#define USB_EP_ATTR_CONTROL                     0b00000000
#define USB_EP_ATTR_ISOCHRONOUS                 0b00000001
#define USB_EP_ATTR_BULK                        0b00000010
#define USB_EP_ATTR_INTERRUPT                   0b00000011

/* table 9-15/9-16: LANGID/UNICODE string descriptor */
struct usb_string_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wString[];
} __attribute__((packed));

/* <https://winprotocoldoc.z19.web.core.windows.net/MS-LCID/%5bMS-LCID%5d.pdf>
*/

#define USB_LANGID_EN_US                        0x0409

/* ----------------------------------------------------------------------------------- */
/* --- USB DEVICE -------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

typedef struct usb_device usb_device;

enum usb_req_result {
    USB_REQ_ERR,
    USB_REQ_HANDLED,
    USB_REQ_DEFER,
};

enum usb_transaction {
    USB_TRANSACTION_IN,
    USB_TRANSACTION_OUT,
    USB_TRANSACTION_SETUP,
};

enum usb_xfer_stage {
    USB_IDLE,
    USB_DATA_IN,  USB_LAST_DATA_IN,  USB_STATUS_IN,
    USB_DATA_OUT, USB_LAST_DATA_OUT, USB_STATUS_OUT,
};

#define MAX_ENDPOINTS               8
#define MAX_USER_EP0_REQ_HANDLER    4
#define MAX_CIB_PACKET_SIZE         64

typedef void (*usb_ep0_req_complete_callback)(usb_device *usb_dev,
                                              struct usb_setup_data *req);

typedef enum usb_req_result (*usb_ep0_req_handler)(usb_device *usb_dev, 
                                                   struct usb_setup_data *req, 
                                                   uint8_t **buf, 
                                                   uint16_t *len,
                                                   usb_ep0_req_complete_callback *cb);

typedef void (*usb_set_config_callback)(usb_device *usb_dev,
                                        uint16_t wValue);

typedef void (*usb_endpoint_callback)(usb_device *usb_dev, 
                                      uint8_t ep);

typedef struct usb_device {

    const struct usb_device_descriptor *dev_desc;
    const struct usb_configuration_descriptor *config;
    const struct usb_string_descriptor * const *str_descs;
    uint8_t  num_str_descs;
    uint16_t status;        /* bus-powered OR self-powered */
    uint8_t  configured;    /* 0 = address/default state, 1 = configured state */

    /* ep0 state machine */
    struct usb_ep0_state {
        enum usb_xfer_stage stage;
        struct usb_setup_data req;
        uint8_t *xfer_buf;
        uint16_t xfer_len;
        int short_xfer;
        usb_ep0_req_complete_callback req_cmpl;
    } ep0;

    struct user_ep0_req_handler {
        usb_ep0_req_handler cb;
        uint8_t type;
        uint8_t type_mask;
    } user_ep0_req_handler[MAX_USER_EP0_REQ_HANDLER];

    usb_endpoint_callback user_ctr_callback[MAX_ENDPOINTS][3];
    usb_set_config_callback user_set_config_callback;

} usb_device;

/* ----------------------------------------------------------------------------------- */
/* --- HELPER MACROS ----------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* if you want strict ISO compliance, use this macro to define a unique struct with a
 * fixed-length wString member for each string descriptor.
 */
#define USB_STRING_DESCRIPTOR(name, ...) \
    enum { name##_char_count = sizeof((uint16_t[]){__VA_ARGS__}) / sizeof(uint16_t) };  \
    struct name##_desc {                                                                \
        uint8_t  bLength;                                                               \
        uint8_t  bDescriptorType;                                                       \
        uint16_t wString[name##_char_count];                                            \
    } __attribute__((packed));                                                          \
    const struct name##_desc name = {                                                   \
        .bLength         = 2 + ((name##_char_count) * 2),                               \
        .bDescriptorType = USB_DT_STRING,                                               \
        .wString         = {__VA_ARGS__}                                                \
    }

/* ----------------------------------------------------------------------------------- */
/* --- USB USER API ------------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */

void usb_enable_isr(void);
usb_device * usb_init(const struct usb_device_descriptor *dev_desc, 
                      const struct usb_configuration_descriptor *config, 
                      const struct usb_string_descriptor * const *str_descs, 
                      int num_string_descs);
void usb_start(usb_device *dev);
void usb_stop(usb_device *dev);
void usb_setup_ep(usb_device *dev, uint8_t addr, uint16_t type, uint16_t max_size, 
                  usb_endpoint_callback ctr_callback);
void usb_handle_event(usb_device *dev);
uint16_t usb_ep_write_packet(usb_device *dev, uint8_t addr, const void *buf, uint16_t len);
uint16_t usb_ep_read_packet(usb_device *dev, uint8_t addr, void *buf, uint16_t len);
extern int usb_register_ep0_req_handler(usb_device *dev, uint8_t type, 
                                        uint8_t type_mask, usb_ep0_req_handler callback);
extern void usb_register_set_config_callback(usb_device *dev, 
                                             usb_set_config_callback callback);

/* ----------------------------------------------------------------------------------- */
/* --- FOR USB_EP0.c ---------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

void usb_set_device_address(usb_device *dev, uint8_t addr);
void usb_reset_endpoints(usb_device *dev);
void usb_set_device_address(usb_device *dev, uint8_t addr);
void usb_ep_set_stall(usb_device *dev, uint8_t addr);
void usb_ep_set_clr_nak(usb_device *dev, uint8_t addr, uint8_t nak);
void usb_setup_acked(usb_device *dev);
void usb_prepare_for_status(usb_device *dev, uint8_t dir);
void usb_status_acked(usb_device *dev, uint8_t dir);

#endif