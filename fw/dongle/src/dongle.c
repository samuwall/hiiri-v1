/********************************************************************
 ** file         : dongle.c
 ** description  : main() 
 **
 **                nRF52840 dongle
 **
 ********************************************************************/

#define PRINT 0

#include <stdint.h>
#include <stddef.h>
#include "device.h"
#include "utils.h"
#include "usb.h"
#include "hid.h"
#include "SEGGER_RTT.h"

static void hid_set_configuration(usb_device *dev, uint16_t wValue);

#define LED_PIN GPIO6

struct mouse_packet {
    uint8_t  LENGTH;
    uint8_t  btn_vbat;
    int16_t  dx;
    int16_t  dy;
    int8_t   wheel;
} __attribute__((packed));

struct dongle_packet {
    uint8_t  LENGTH;
    uint16_t cc;
    uint16_t dpi;
} __attribute__((packed));

enum radio_state {
    STATE_RX,
    STATE_TX
};

volatile enum radio_state radio_state    = STATE_RX;
volatile struct mouse_packet  rx_pkt     = {0};
volatile struct mouse_packet  mouse_pkt  = {0};
volatile struct dongle_packet dongle_pkt = {.LENGTH = 4, .dpi = 800};

/* our handle (ptr) to the device alloc'd in `usb.c` */
static usb_device *usb_dev;

/* byte 0:
 *   REPORT_COUNT (2), REPORT_SIZE(1)  = button 1,2  = 2 bits
 *   REPORT_COUNT (1), REPORT_SIZE(6)  = padding     = 6 bits
 * bytes 1-6: 
 *   REPORT_COUNT (3), REPORT_SIZE(16) = X, Y, Wheel = 3 * 2 bytes
 * bytes 7:
 *   REPORT_COUNT (1), REPORT_SIZE(8)  = padding     = 8 bits
 */
const uint8_t hid_mouse_report_descriptor[] = {
    0x05, 0x01,         /* USAGE_PAGE (Generic Desktop)         */
    0x09, 0x02,         /* USAGE (Mouse)                        */
    0xa1, 0x01,         /* COLLECTION (Application)             */
    0x09, 0x01,         /*   USAGE (Pointer)                    */
    0xa1, 0x00,         /*   COLLECTION (Physical)              */
    0x05, 0x09,         /*     USAGE_PAGE (Button)              */
    0x19, 0x01,         /*     USAGE_MINIMUM (Button 1)         */
    0x29, 0x02,         /*     USAGE_MAXIMUM (Button 2)         */
    0x15, 0x00,         /*     LOGICAL_MINIMUM (0)              */
    0x25, 0x01,         /*     LOGICAL_MAXIMUM (1)              */
    0x95, 0x02,         /*     REPORT_COUNT (2)                 */
    0x75, 0x01,         /*     REPORT_SIZE (1)                  */
    0x81, 0x02,         /*     INPUT (Data,Var,Abs)             */
    0x95, 0x01,         /*     REPORT_COUNT (1)                 */
    0x75, 0x06,         /*     REPORT_SIZE (6)                  */
    0x81, 0x01,         /*     INPUT (Cnst,Ary,Abs)             */
    0x05, 0x01,         /*     USAGE_PAGE (Generic Desktop)     */
    0x09, 0x30,         /*     USAGE (X)                        */
    0x09, 0x31,         /*     USAGE (Y)                        */
    0x09, 0x38,         /*     USAGE (Wheel)                    */
    0x16, 0x01, 0x80,   /*     LOGICAL_MINIMUM (-32767)         */
    0x26, 0xff, 0x7f,   /*     LOGICAL_MAXIMUM (32767)          */
    0x95, 0x03,         /*     REPORT_COUNT (3)                 */
    0x75, 0x10,         /*     REPORT_SIZE (16)                 */
    0x81, 0x06,         /*     INPUT (Data,Var,Rel)             */
    0x95, 0x01,         /*     REPORT_COUNT (1)                 */
    0x75, 0x08,         /*     REPORT_SIZE (8)                  */
    0x81, 0x01,         /*     INPUT (Cnst,Ary,Abs)             */
    0xc0,               /*   END_COLLECTION                     */
    0x09, 0x3c,         /*   USAGE (Motion Wakeup)              */
    0xc0                /* END_COLLECTION                       */
};

/* PS1.11 p.860 claims interrupt/bulk transactions
 * must be multiples of 4 bytes and 32-bit aligned
 */
struct hid_mouse_report {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int16_t wheel;
    uint8_t padding;
} __attribute__((packed, aligned(4)));

const struct usb_device_descriptor device_descriptor = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x1915,
    .idProduct          = 0x572B,
    .bcdDevice          = 0x0200,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,
    .bNumConfigurations = 1,
};

struct config_block {
    struct usb_configuration_descriptor config;
    struct usb_interface_descriptor     if0;
    struct usb_hid_descriptor           if0_hid;
    struct usb_endpoint_descriptor      if0_hid_ep;
} __attribute__((packed));

struct config_block hid_mouse_cfg_block = {
    
    .config = {
        .bLength                = USB_DT_CONFIGURATION_SIZE,
        .bDescriptorType        = USB_DT_CONFIGURATION,
        .wTotalLength           = sizeof(struct usb_configuration_descriptor) +
                                  sizeof(struct usb_interface_descriptor) +
                                  sizeof(struct usb_hid_descriptor) +
                                  sizeof(struct usb_endpoint_descriptor),
        .bNumInterfaces         = 1,
        .bConfigurationValue    = 1,
        .iConfiguration         = 0,
        .bmAttributes           = 0x80,
        .bMaxPower              = 0x32,
    },

    .if0 = {
        .bLength                = USB_DT_INTERFACE_SIZE,
        .bDescriptorType        = USB_DT_INTERFACE,
        .bInterfaceNumber       = 0,
        .bAlternateSetting      = 0,
        .bNumEndpoints          = 1,
        .bInterfaceClass        = USB_CLASS_HID,
        .bInterfaceSubClass     = USB_HID_SUBCLASS_NO,
        .bInterfaceProtocol     = USB_HID_INTERFACE_PROTOCOL_NONE,
        .iInterface             = 0,
    },

    .if0_hid = {
        .bLength                 = sizeof(struct usb_hid_descriptor),
        .bDescriptorType         = USB_HID_DT_HID,
        .bcdHID                  = 0x0111,
        .bCountryCode            = 0,
        .bNumDescriptors         = 1,
        .bReportDescriptorType   = USB_HID_DT_REPORT,
        .wReportDescriptorLength = sizeof(hid_mouse_report_descriptor),
    },

    .if0_hid_ep = {
        .bLength                = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType        = USB_DT_ENDPOINT,
        .bEndpointAddress       = 0x81,
        .bmAttributes           = USB_EP_ATTR_INTERRUPT,
        .wMaxPacketSize         = sizeof(struct hid_mouse_report),
        .bInterval              = 0x01,
    }

};

const struct usb_string_descriptor str_langid = {
    .bLength            = 4,
    .bDescriptorType    = USB_DT_STRING,
    .wString            = { USB_LANGID_EN_US },
};

const struct usb_string_descriptor str_mfr = {
    /* bLength = bLength + bDescriptorType + (UTF-16 chars * 2)
     *         =    1    +        1        + (      9      * 2)
     *         =   20
     */
    .bLength            = 20,
    .bDescriptorType    = USB_DT_STRING,
    .wString            = {
        'H','i','i','r','i',' ','C','o','.'
    },
};

const struct usb_string_descriptor str_product = {
    .bLength            = 20,
    .bDescriptorType    = USB_DT_STRING,
    .wString            = {
        'H','I','D',' ','M','o','u','s','e'
    }
};

const struct usb_string_descriptor str_serial = {
    .bLength            = 12,
    .bDescriptorType    = USB_DT_STRING,
    .wString            = {
        '6','9','4','2','0'
    }
};

const struct usb_string_descriptor * const strings[] = {
    &str_langid,
    &str_mfr,
    &str_product,
    &str_serial,
};

static void power_setup(void) {
    POWER->TASKS_CONSTLAT = 1;
}

static void clock_setup(void) {
    CLOCK->TASKS_HFCLKSTART = 1;
    while (!(CLOCK->EVENTS_HFCLKSTARTED));
}

static void timer_setup(void) {

    TIMER0->TASKS_STOP  = 1;
    TIMER0->TASKS_CLEAR = 1;
    TIMER0->MODE        = TIMER_MODE_MODE_Timer;
    TIMER0->BITMODE     = TIMER_BITMODE_BITMODE_32Bit;
    TIMER0->PRESCALER   = 4;

    TIMER0->TASKS_START = 1;
}

static void radio_setup(void) {

    RADIO->MODE = RADIO_MODE_MODE_Nrf_2Mbit;
    RADIO->MODECNF0 = RADIO_MODECNF0_RU_Fast | RADIO_MODECNF0_DTX_Center;

    RADIO->PCNF0 = (8 << RADIO_PCNF0_LFLEN_Shft)  |
                   (0 << RADIO_PCNF0_S0LEN_Shft)  |
                   (0 << RADIO_PCNF0_S1LEN_Shft)  |
                   (RADIO_PCNF0_S1INCL_Automatic) |
                   (RADIO_PCNF0_PLEN_8bit);

    RADIO->PCNF1 = (8 << RADIO_PCNF1_MAXLEN_Shft)   |
                   (0 << RADIO_PCNF1_STATLEN_Shft)  |
                   (3 << RADIO_PCNF1_BALEN_Shft)    |
                   (RADIO_PCNF1_ENDIAN_Little)      |
                   (RADIO_PCNF1_WHITEEN_Enabled);
    
    RADIO->DATAWHITEIV = 0x69;
    RADIO->CRCCNF = (2 << RADIO_CRCCNF_LEN_Shft) |
                    (RADIO_CRCCNF_SKIPADDR_Skip);
    RADIO->CRCPOLY = 0x001685F1;
    RADIO->CRCINIT = 0x000656E9;

    /* 2402 MHz, 0dBm */
    RADIO->FREQUENCY = 2;
    RADIO->TXPOWER   = 0;

    /* on-air addresses
     * = [PREFIX byte] + [BALEN bytes of BASE]
     */
    RADIO->BASE0   = 0x00440967;
    RADIO->BASE1   = 0x00764635;
    RADIO->PREFIX0 = 0x2A5F;

    /* logical addresses */
    RADIO->TXADDRESS   = 1;
    RADIO->RXADDRESSES = RADIO_RXADDRESSES_ADDR0_Enabled;

    /* shortcuts */
    RADIO->SHORTS = RADIO_SHORTS_READY_START_
                  | RADIO_SHORTS_END_DISABLE_;

    /* enable interrupt for packet sent/received */
    RADIO->INTENSET = RADIO_INTENSET_DISABLED_Set;
    NVIC->ISER[NVIC_RADIO_IRQ / 32] = (1 << (NVIC_RADIO_IRQ % 32));

}

static void set_polling_rate(usb_device *dev, struct usb_setup_data *req) {

    (void)dev;

    uint8_t bInterval = req->wIndex;
    hid_mouse_cfg_block.if0_hid_ep.bInterval = bInterval;

    #if DBG >= 1
    SEGGER_RTT_printf(0, "\nRESETTING DEVICE WITH NEW POLLING RATE!\nbInterval = %d\n\n", 
                         bInterval);
    #endif

    /* arbitrary delay to allow status stage to be acknowledged.. 
     * no way to detect an acked status in the nordic periph
     */
    for (volatile uint32_t i = 0; i < 50000; i++);

    usb_stop(usb_dev);
    usb_dev = usb_init(&device_descriptor, &hid_mouse_cfg_block.config,
                        strings, sizeof(strings) / sizeof(strings[0]));
    usb_register_set_config_callback(usb_dev, hid_set_configuration);
    usb_start(usb_dev);

}

static enum usb_req_result
handle_set_mousesettings(usb_device *dev, struct usb_setup_data *req, uint8_t **buf, 
                         uint16_t *len, usb_ep0_req_complete_callback *cb) {
    (void)dev;
    (void)buf;
    (void)len;

    /* custom 'vendor-specific' request for setting DPI/pollrate */
    if ((req->bmRequestType != 0b01000000) || (req->bRequest != 0x01)) {
        return USB_REQ_DEFER;
    }

    if (req->wValue > 0) {
        #if DBG >= 1
        SEGGER_RTT_printf(0, "set dpi: %d\n", req->wValue);
        #endif
        dongle_pkt.dpi = req->wValue;
    }

    if (req->wIndex > 0) {
        *cb = set_polling_rate;
    }

    return USB_REQ_HANDLED;
}

static enum usb_req_result
handle_get_mousevbat(usb_device *dev, struct usb_setup_data *req, uint8_t **buf,
                     uint16_t *len, usb_ep0_req_complete_callback *cb) {
    (void)dev;
    (void)cb;

    /* custom 'vendor-specific' request for getting mouse vbat */
    if ((req->bmRequestType != 0b11000000) || req->bRequest != 0x01) {
        return USB_REQ_DEFER;
    }

    /* point ep0 xfer buf to data requested */
    *buf = (uint8_t *) &mouse_pkt.btn_vbat;
    *len = sizeof(mouse_pkt.btn_vbat);

    return USB_REQ_HANDLED;
}

static enum usb_req_result
handle_hid_get_report_descriptor(usb_device *dev, struct usb_setup_data *req, uint8_t **buf, 
                                 uint16_t *len, usb_ep0_req_complete_callback *cb) {
    (void)dev;
    (void)cb;

    if ((req->bmRequestType != 0b10000001) || (req->bRequest != USB_REQ_GET_DESCRIPTOR)
       || (req->wValue != (USB_HID_DT_REPORT << 8))) {
        return USB_REQ_DEFER; /* defer handling to std handlers or subsequent user cb */
    }

    /* point ep0 xfer buf to data requested */
    *buf = (uint8_t *)hid_mouse_report_descriptor;
    *len = sizeof(hid_mouse_report_descriptor);

    return USB_REQ_HANDLED;
}

static void send_hid_report(usb_device *dev, uint8_t ep) {

    (void)ep;
    static struct hid_mouse_report report = {0};

    TIMER0->TASKS_CAPTURE[0] = 1;
    TIMER0->TASKS_CLEAR = 1;

    report.buttons = mouse_pkt.btn_vbat;
    report.x       = mouse_pkt.dx;
    report.y       = mouse_pkt.dy;
    report.wheel   = mouse_pkt.wheel;

    usb_ep_write_packet(dev, 0x81, &report, sizeof(report));

    #if PRINT 
    SEGGER_RTT_printf(0, "USB: %d\n", TIMER0->CC[0]);
    #endif

}

static void hid_set_configuration(usb_device *dev, uint16_t wValue) {

    (void)wValue;

    usb_setup_ep(dev, 0x81, USB_EP_ATTR_INTERRUPT, sizeof(struct hid_mouse_report), send_hid_report);

    usb_register_ep0_req_handler(dev, 
        USB_REQ_TYPE_IN        | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_DIRECTION | USB_REQ_TYPE_TYPE     | USB_REQ_TYPE_RECIPIENT,
        handle_hid_get_report_descriptor);

    usb_register_ep0_req_handler(dev, 
        USB_REQ_TYPE_OUT       | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE,
        USB_REQ_TYPE_DIRECTION | USB_REQ_TYPE_TYPE   | USB_REQ_TYPE_RECIPIENT,
        handle_set_mousesettings);
    
    usb_register_ep0_req_handler(dev, 
        USB_REQ_TYPE_IN        | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE,
        USB_REQ_TYPE_DIRECTION | USB_REQ_TYPE_TYPE   | USB_REQ_TYPE_RECIPIENT,
        handle_get_mousevbat);

    /* fill ep1 tx buffer with first report; start chain of CTR IN events */
    send_hid_report(dev, 0x81);

    /* device configured .. start receiving mouse packets */
    RADIO->PACKETPTR  = (uint32_t) &rx_pkt;
    RADIO->TASKS_RXEN = 1;

}

int main(void) {

    power_setup();
    clock_setup();
    timer_setup();
    radio_setup();
    usb_enable_isr();

    /* receive usb device handler */
    usb_dev = usb_init(&device_descriptor, &hid_mouse_cfg_block.config, 
                        strings, sizeof(strings) / sizeof(strings[0]));

    /* register the func that will run when the host sends the `set_configuration` request */
    usb_register_set_config_callback(usb_dev, hid_set_configuration);

    /* enable peripheral, start enumeration */
    usb_start(usb_dev);

    for (;;) {
        
    }

}

void usbd_isr(void) {
    usb_handle_event(usb_dev);
}

void radio_isr(void) {

    RADIO->EVENTS_DISABLED = 0;

    if (radio_state == STATE_RX) {

        TIMER0->TASKS_CAPTURE[1] = 1;
        dongle_pkt.cc = ((TIMER0->CC[0]) - (TIMER0->CC[1])) - 100;

        RADIO->PACKETPTR = (uint32_t) &dongle_pkt;
        RADIO->TASKS_TXEN = 1;

        if (RADIO->CRCSTATUS) {
            mouse_pkt = rx_pkt;
        }

        radio_state = STATE_TX;
        P0->DIRSET = LED_PIN;
    }
    else {

        RADIO->PACKETPTR = (uint32_t) &rx_pkt;
        RADIO->TASKS_RXEN = 1;

        #if PRINT
        SEGGER_RTT_printf(0, "RADIO: %3d, pkt.cc = %d\n", TIMER0->CC[1], dongle_pkt.cc);
        #endif

        radio_state = STATE_RX;
        P0->DIRCLR = LED_PIN;
    }

}
