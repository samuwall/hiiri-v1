/********************************************************************
 ** file         : usb.c
 ** description  : nRF52840 USBFS device hw peripheral driver
 **
 **
 ********************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include "device.h"
#include "utils.h"
#include "usb.h"
#include "usb_ep0.h"

#include "SEGGER_RTT.h"

static void usbd_errata_no187(void);
static void usbd_errata_no199(uint8_t start);

static usb_device nrf_usbfs_dev;
static uint8_t dma_buf[MAX_CIB_PACKET_SIZE];
static atomic_flag dma_busy = ATOMIC_FLAG_INIT;

void usb_enable_isr(void) {
    NVIC->ISER[NVIC_USBD_IRQ / 32] = (1 << (NVIC_USBD_IRQ % 32));
}

usb_device * usb_init(const struct usb_device_descriptor *dev_desc, 
                      const struct usb_configuration_descriptor *config, 
                      const struct usb_string_descriptor * const *str_descs, 
                      int num_string_descs) {

    usb_device *usb_dev     = &nrf_usbfs_dev;
    usb_dev->dev_desc       = dev_desc;
    usb_dev->config         = config;
    usb_dev->str_descs      = str_descs;
    usb_dev->num_str_descs  = num_string_descs;
    usb_dev->status         = (config->bmAttributes & USB_CFG_ATTR_SELF_POWERED) ? 1 : 0;

    usb_dev->user_ctr_callback[0][USB_TRANSACTION_SETUP] = usb_ep0_setup;
    usb_dev->user_ctr_callback[0][USB_TRANSACTION_OUT]   = usb_ep0_out;
    usb_dev->user_ctr_callback[0][USB_TRANSACTION_IN]    = usb_ep0_in;

    usb_dev->user_set_config_callback = NULL;

    for (int i = 0; i < MAX_USER_EP0_REQ_HANDLER; i++) {
        usb_dev->user_ep0_req_handler[i].cb = NULL;
    }

    return usb_dev;
}

void usb_start(usb_device *dev) {

    (void)dev;

    /* verify VBUS */
    while (!(POWER->EVENTS_USBDETECTED));
    
    /* enable peripheral */
    usbd_errata_no187();

    /* verify usb phy power */
    while (!(POWER->EVENTS_USBPWRRDY));

    /* enable interrupts */
    USBD->INTENSET = USBD_INTEN_USBRESET_  | USBD_INTEN_EP0DATADONE_ |
                     USBD_INTEN_EP0SETUP_  | USBD_INTEN_EPDATA_      |
                     USBD_INTEN_ENDEP_;

    /* present FS device */
    USBD->USBPULLUP = 1;

}

void usb_stop(usb_device *dev) {

    (void)dev;

    /* disable interrupts */
    USBD->INTEN = 0x0;

    /* disable peripheral */
    USBD->ENABLE = 0;
    while (USBD->ENABLE);

    /* disable pull-up */
    USBD->USBPULLUP = 0;

}

void usb_setup_ep(usb_device *dev, uint8_t addr, uint16_t type, uint16_t max_size, 
                  usb_endpoint_callback ctr_callback) {

    (void)dev;
    (void)type;
    (void)max_size;

    uint8_t dir = (addr >> 7) & 0b1;
    uint8_t ep  =  addr & 0x7F;

    if (dir == 1) {
        USBD->EPINEN |= (1 << ep);
        if (ctr_callback) {
            dev->user_ctr_callback[ep][USB_TRANSACTION_IN] = ctr_callback;
        }
    }
    
    if (dir == 0) {
        USBD->EPOUTEN |= (1 << ep);
        USBD->SIZE.EPOUT[ep] = 0;
        if (ctr_callback) {
            dev->user_ctr_callback[ep][USB_TRANSACTION_OUT] = ctr_callback;
        }
    }

}

void usb_ep_set_stall(usb_device *dev, uint8_t addr) {

    (void)dev;
    (void)addr;

    USBD->TASKS_EP0STALL = 1;

    #if DBG >= 1
    SEGGER_RTT_printf(0, "TASKS_EP0STALL = 1\n");
    #endif

}

void usb_ep_set_clr_nak(usb_device *dev, uint8_t addr, uint8_t nak) {

    (void)dev;
    (void)addr;
    (void)nak;

    /* handled by hw. TX/RX are always NAK until we
     * set either TASKS_STARTEPIN/OUT[] or TASKS_EP0STATUS
     * nRF52840 PS1.11 p. 857
     */
}

void usb_setup_acked(usb_device *dev) {

    usb_ep_set_clr_nak(dev, 0, 1);
    dev->ep0.req_cmpl = NULL;

}

void usb_prepare_for_status(usb_device *dev, uint8_t dir) {

    (void)dir;

    USBD->TASKS_EP0STATUS = 1;

    #if DBG >= 1
    SEGGER_RTT_printf(0, "TASKS_EP0STATUS = 1\n");
    #endif

    if (dev->ep0.req_cmpl) {
        dev->ep0.req_cmpl(dev, &dev->ep0.req);
    }

}

void usb_status_acked(usb_device *dev, uint8_t dir) {

    (void)dev;
    (void)dir;

    /* do nothing. st usbfs for example would 
     * have to read the ZLP here for a STATUS_OUT
     * ctr, but for us this is all taken care of 
     * with TASKS_EP0STATUS.. we dont even reach
     * this code with an nRF52..
     */
}

void usb_set_device_address(usb_device *dev, uint8_t addr) {

    (void)dev;
    (void)addr;

    /* for nrf5x, do nothing */

}

uint16_t usb_ep_write_packet(usb_device *dev, uint8_t addr, const void *buf, uint16_t len) {

    (void)dev;
    uint8_t ep = addr & 0x7F;

    /* only 1 dma xfer at a time (see note 1) */
    if (atomic_flag_test_and_set(&dma_busy)) {
        return 0xFFFF;
    }

    if (buf < (void *) 0x100000) {
        /* buf in FLASH, must copy to RAM */
        my_memcpy(dma_buf, buf, len);
        USBD->EPIN[ep].PTR    = (uint32_t) dma_buf;
        USBD->EPIN[ep].MAXCNT = len;
    }
    else {
        /* buf already in RAM */
        USBD->EPIN[ep].PTR    = (uint32_t) buf;
        USBD->EPIN[ep].MAXCNT = len;
    }

    usbd_errata_no199(1);

    USBD->TASKS_STARTEPIN[ep] = 1;
    return len;
}

uint16_t usb_ep_read_packet(usb_device *dev, uint8_t addr, void *buf, uint16_t len) {

    (void)dev;
    uint8_t ep = addr & 0x7F;

    if (atomic_flag_test_and_set(&dma_busy)) { 
        return 0xFFFF;
    }

    len = MIN(USBD->SIZE.EPOUT[ep], len);
    USBD->EPOUT[ep].PTR    = (uint32_t) buf;
    USBD->EPOUT[ep].MAXCNT = len;

    usbd_errata_no199(1);

    USBD->TASKS_STARTEPOUT[ep] = 1;
    return len;
}

void usb_reset_endpoints(usb_device *dev) {

    (void)dev;

    /* disable all EPs except ep0 */
    USBD->EPINEN  = 0x01;
    USBD->EPOUTEN = 0x01;

    /* set DTOGGLE/EPSTALL of all EPs to 0? 
     */
}

static void usb_reset(usb_device * dev) {

    /* default state */
    dev->configured = 0;

    /* in case reset interrupted dma */
    usbd_errata_no199(0);
    atomic_flag_clear(&dma_busy);

    /* ep0 is set up by default, and hw already handles
     * setting DADDR back to 0 and everything..
     */
    USBD->EPINEN  = 0x01;
    USBD->EPOUTEN = 0x01;

}

void usb_handle_event(usb_device *dev) {

    uint32_t events = 0;
    volatile uint32_t *events_reg = &USBD->EVENTS_USBRESET;
    
    for (uint8_t i = 0; i < USBD_INTEN_EPDATA_Shft + 1; i++) {
        if (events_reg[i]) {
            events |= (1 << i);
            events_reg[i] = 0;
        }
    }

    if (events & USBD_INTEN_ENDEP_) {

        #if DBG >= 1
        SEGGER_RTT_printf(0, "ENDEP\n");
        #endif

        usbd_errata_no199(0);
        atomic_flag_clear(&dma_busy);
        
    }

    if (events & USBD_INTEN_USBRESET_) {

        #if DBG >= 1
        SEGGER_RTT_printf(0, "RESET\n");
        #endif

        usb_reset(dev);
        return;
    }

    if (events & USBD_INTEN_EP0DATADONE_) {

        #if DBG >= 1
        SEGGER_RTT_printf(0, "EP0DATADONE\n");
        #endif

        uint8_t stage = dev->ep0.stage;

        if (stage == USB_DATA_IN || stage == USB_LAST_DATA_IN) {
            dev->user_ctr_callback[0][USB_TRANSACTION_IN] (dev, 0);
        }
        else if (stage == USB_DATA_OUT || stage == USB_LAST_DATA_OUT) {
            dev->user_ctr_callback[0][USB_TRANSACTION_OUT] (dev, 0);
        }
        else {
            #if DBG >= 1
            SEGGER_RTT_printf(0, "stage: %d\n", stage);
            #endif
        }
        
    }

    if (events & USBD_INTEN_EPDATA_) {

        #if DBG >= 1
        SEGGER_RTT_printf(0, "EPDATA\n");
        #endif

        uint32_t data_status = USBD->EPDATASTATUS;

        for (uint8_t ep = 1; ep < 8; ep++) {
            if (data_status & (1 << ep)) {
                dev->user_ctr_callback[ep][USB_TRANSACTION_IN] (dev, ep | 0x80);
                USBD->EPDATASTATUS = (1 << ep);
            }
            if (data_status & (1 << (ep + 16))) {
                dev->user_ctr_callback[ep][USB_TRANSACTION_OUT] (dev, ep);
                USBD->EPDATASTATUS = (1 << (ep + 16));
            }
        }
    }

    if (events & USBD_INTEN_EP0SETUP_) {

        #if DBG >= 1
        SEGGER_RTT_printf(0, "EP0SETUP\n");
        #endif

        dev->ep0.req.bmRequestType  = USBD->BMREQUESTTYPE;
        dev->ep0.req.bRequest       = USBD->BREQUEST;
        dev->ep0.req.wValue         = (USBD->WVALUEH  << 8) | (USBD->WVALUEL);
        dev->ep0.req.wIndex         = (USBD->WINDEXH  << 8) | (USBD->WINDEXL);
        dev->ep0.req.wLength        = (USBD->WLENGTHH << 8) | (USBD->WLENGTHL);

        dev->user_ctr_callback[0][USB_TRANSACTION_SETUP] (dev, 0);
    }

}

static void usbd_errata_no187(void) {

    /* nrf52840 rev. 3 errata v1.3
     * conditions:
     *      most recent reset is soft, like
     *      after flashing new firmware
     * symptoms:
     *      after writing to USBD->ENABLE, 
     *      EVENTCAUSE_READY is never updated
     * workaround:
     *      call this function in place 
     *      of `USBD->ENABLE`
     */

    *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    *(volatile uint32_t *)0x4006ED14 = 0x00000003;
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    USBD->ENABLE = 1;
    while (!(USBD->EVENTCAUSE & USBD_EVENTCAUSE_READY_));
    USBD->EVENTCAUSE &= ~USBD_EVENTCAUSE_READY_;
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    *(volatile uint32_t *)0x4006ED14 = 0x00000000;
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;
}

static void usbd_errata_no199(uint8_t start) {

    /* nrf52840 rev. 3 errata v1.3
     * conditions:
     *      easyDMA xfer is in progress
     * symptoms:
     *      USBD tasks cannot be used
     * workaround:
     *      call this function with `start==1` 
     *      when starting a DMA xfer and with
     *      `start==0` when that xfer is over
     */

    if (start) {
        *(volatile uint32_t *)0x40027C1C = 0x82;
    }
    else {
        *(volatile uint32_t *)0x40027C1C = 0x00;
    }
}

/* note 1 : `dma_busy` 
 *
 *          PS1.11 p.856 states that "only a single EasyDMA transfer can take place
 *          in USBD at any time"..
 * 
 *          as such, we consult `dma_busy` before starting any EPIN/OUT task.
 *          it gets set when `TASKS_STARTEPIN/OUT` is called and cleared upon
 *          receiving any `ENDEPIN/OUT` event.
 * 
 *          atomic in case someone uses write/read from both thread/isr context.. 
 * 
 * note 2 : RTT_printf vs. RTT_WriteString
 * 
 *          changing all eligible printfs to WriteString resulted in the same code size
 *          (-Os), so I just kept the printfs
 * 
 */

