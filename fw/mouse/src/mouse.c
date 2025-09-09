/********************************************************************
 ** file         : mouse.c
 ** description  : main() 
 **
 **                nrf52 non-blocking mouse
 **
 ********************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "device.h"
#include "delay.h"
#include "paw3395.h"
#include "utils.h"

#define RX_TIMEOUT_US   200        /* 200us */
#define VBAT_INTERVAL   10000000   /* 10s   */

#define LED_PIN         7
#define L_NO_PIN        8
#define L_NC_PIN        29
#define R_NO_PIN        15
#define R_NC_PIN        14
#define ENC_A_PIN       20
#define ENC_B_PIN       18
#define NCS_PIN         0
#define MISO_PIN        1
#define MOSI_PIN        4
#define SCK_PIN         5
#define MOTION_PIN      6

#define CH_L_NO         0
#define CH_L_NC         1
#define CH_R_NO         2
#define CH_R_NC         3

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

struct radio_ctx {
    enum {
        RADIO_STATE_DISABLED,
        RADIO_STATE_TX,
        RADIO_STATE_TXRU,
        RADIO_STATE_RX,
        RADIO_STATE_RXTO_RXDISABLE,
    } state;
};

struct spim_ctx {
    uint8_t active;
    uint8_t ready;
    enum {
        SPIM_STATE_TX,
        SPIM_STATE_RX,
    } state;
};

struct comp_ctx {
    uint8_t active;
    uint8_t ladder;
};

volatile struct mouse_packet  mouse_pkt  = {.LENGTH = 6};
volatile struct dongle_packet dongle_pkt = {0};
volatile struct radio_ctx     radio_ctx  = {0};
volatile struct spim_ctx      spim_ctx   = {0};
volatile struct comp_ctx      comp_ctx   = {0};

volatile uint8_t  paw_data[BURST_SIZE] = {0};
volatile uint32_t elapsed_us = VBAT_INTERVAL;
volatile uint16_t curr_dpi   = 0;
volatile uint8_t  vbat       = 0;
volatile uint8_t  op_mode    = 0;
volatile uint8_t  l_click    = 0;
volatile uint8_t  r_click    = 0;

static void power_setup(void) {
    POWER->TASKS_CONSTLAT = 1;
}

static void clock_setup(void) {
    CLOCK->TASKS_HFCLKSTART = 1;
    while (!(CLOCK->EVENTS_HFCLKSTARTED));
    CLOCK->EVENTS_HFCLKSTARTED = 0;
}

static void timer_setup(void) {

    /* delay timer */
    TIMER0->TASKS_STOP  = 1;
    TIMER0->TASKS_CLEAR = 1;
    TIMER0->MODE        = TIMER_MODE_MODE_Timer;
    TIMER0->BITMODE     = TIMER_BITMODE_BITMODE_32Bit;
    TIMER0->PRESCALER   = 4; /* 16MHz / 2^4 = 1MHz */
    NVIC->ISER[NVIC_TIMER0_IRQ / 32] = (1 << (NVIC_TIMER0_IRQ % 32));

    /* isr timer */
    TIMER1->TASKS_STOP  = 1;
    TIMER1->TASKS_CLEAR = 1;
    TIMER1->MODE        = TIMER_MODE_MODE_Timer;
    TIMER1->BITMODE     = TIMER_BITMODE_BITMODE_32Bit;
    TIMER1->PRESCALER   = 4;

    TIMER1->CC[0]       = 1000;
    TIMER1->CC[1]       = 0xFFFFFFFF;
    TIMER1->SHORTS      = TIMER_SHORTS_COMPARE0_STOP_Enabled
                        | TIMER_SHORTS_COMPARE0_CLEAR_Enabled;

    TIMER1->INTENSET    = TIMER_INTENSET_COMPARE0_Set
                        | TIMER_INTENSET_COMPARE1_Set;

    NVIC->ISER[NVIC_TIMER1_IRQ / 32] = (1 << (NVIC_TIMER1_IRQ % 32));

}

static void gpio_setup(void) {

    uint8_t pins[] = {L_NO_PIN, L_NC_PIN, R_NO_PIN, R_NC_PIN,
                      ENC_A_PIN, ENC_B_PIN};
    for (uint8_t i = 0; i < ARR_SIZE(pins); i++) {
        P0->PIN_CNF[pins[i]] = GPIO_PIN_CNF_DIR_Input
                             | GPIO_PIN_CNF_INPUT_Connect
                             | GPIO_PIN_CNF_PULL_Pullup
                             | GPIO_PIN_CNF_DRIVE_S0S1
                             | GPIO_PIN_CNF_SENSE_Disabled;
    }

}

static void gpiote_setup(void) {

    /* CH 0: L_NO ; trigger on first switch press */
    GPIOTE->CONFIG[CH_L_NO] = GPIOTE_CONFIG_MODE_Event
                            | GPIOTE_CONFIG_POLARITY_HiToLo
                            | (L_NO_PIN << GPIOTE_CONFIG_PSEL_Shft);

    /* CH 1: L_NC */
    GPIOTE->CONFIG[CH_L_NC] = 0;

    /* CH 2: R_NO ; trigger on first switch press */
    GPIOTE->CONFIG[CH_R_NO] = GPIOTE_CONFIG_MODE_Event
                            | GPIOTE_CONFIG_POLARITY_HiToLo
                            | (R_NO_PIN << GPIOTE_CONFIG_PSEL_Shft);

    /* CH 3: R_NC */
    GPIOTE->CONFIG[CH_R_NC] = 0;

    /* clear any spurious events */
    GPIOTE->EVENTS_IN[CH_L_NO] = 0;
    GPIOTE->EVENTS_IN[CH_L_NC] = 0;
    GPIOTE->EVENTS_IN[CH_R_NO] = 0;
    GPIOTE->EVENTS_IN[CH_R_NC] = 0;

    /* enable interrupts */
    GPIOTE->INTENSET = (1 << CH_L_NO) | (1 << CH_L_NC)
                     | (1 << CH_R_NO) | (1 << CH_R_NC);

    /* enable in NVIC */
    NVIC->ISER[NVIC_GPIOTE_IRQ / 32] = (1 << (NVIC_GPIOTE_IRQ % 32));

}

static void qdec_setup(void) {

    QDEC->SAMPLEPER = QDEC_SAMPLEPER_SAMPLEPER_128us;
    QDEC->DBFEN     = QDEC_DBFEN_DBFEN_Disabled;

    QDEC->PSEL.A    = (ENC_A_PIN << QDEC_PSEL_PIN_Shft) | (QDEC_PSEL_CONNECT_Connected);
    QDEC->PSEL.B    = (ENC_B_PIN << QDEC_PSEL_PIN_Shft) | (QDEC_PSEL_CONNECT_Connected);

    QDEC->ENABLE    = QDEC_ENABLE_ENABLE_Enabled;
    QDEC->TASKS_START = 1;

}

static void spi_setup(void) {
    
    uint8_t out_pins[] = {NCS_PIN, MOSI_PIN, SCK_PIN};
    for (uint8_t i = 0; i < ARR_SIZE(out_pins); i++) {
        P0->PIN_CNF[out_pins[i]] = GPIO_PIN_CNF_DIR_Output
                                 | GPIO_PIN_CNF_INPUT_Disconnect
                                 | GPIO_PIN_CNF_PULL_Disabled
                                 | GPIO_PIN_CNF_DRIVE_S0S1
                                 | GPIO_PIN_CNF_SENSE_Disabled;
    }

    uint8_t in_pins[] = {MISO_PIN};
    for (uint8_t i = 0; i < ARR_SIZE(in_pins); i++) {
        P0->PIN_CNF[in_pins[i]] = GPIO_PIN_CNF_DIR_Input
                                | GPIO_PIN_CNF_INPUT_Connect
                                | GPIO_PIN_CNF_PULL_Disabled
                                | GPIO_PIN_CNF_DRIVE_S0S1
                                | GPIO_PIN_CNF_SENSE_Disabled;
    }

    P0->OUTSET = (1 << NCS_PIN) | (1 << SCK_PIN);
    P0->OUTCLR = (1 << MOSI_PIN);

    SPIM0->PSEL.MISO = (MISO_PIN << SPIM_PSEL_PIN_Shft) | (SPIM_PSEL_CONNECT_Connected);
    SPIM0->PSEL.MOSI = (MOSI_PIN << SPIM_PSEL_PIN_Shft) | (SPIM_PSEL_CONNECT_Connected);
    SPIM0->PSEL.SCK  = (SCK_PIN  << SPIM_PSEL_PIN_Shft) | (SPIM_PSEL_CONNECT_Connected);

    /* mode 3 */
    SPIM0->CONFIG = SPIM_CONFIG_ORDER_MsbFirst 
                  | SPIM_CONFIG_CPHA_Trailing
                  | SPIM_CONFIG_CPOL_ActiveLow;
    
    SPIM0->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_4Mbps;

    NVIC->ISER[NVIC_SPI0_SPIM0_SPIS0_TWI0_TWIM0_TWIS0_IRQ / 32] = 
        (1 << (NVIC_SPI0_SPIM0_SPIS0_TWI0_TWIM0_TWIS0_IRQ % 32));

    SPIM0->ENABLE = SPIM_ENABLE_ENABLE_Enabled;

}

static void radio_setup(void) {

    /* 2Mb nordic proprietary */
    RADIO->MODE = RADIO_MODE_MODE_Nrf_2Mbit;

    /* nrf52 fast ramp up */
    RADIO->MODECNF0 = RADIO_MODECNF0_RU_Fast | RADIO_MODECNF0_DTX_Center;

    /* 8-bit LENGTH field
     * 8-bit preamble 
     */
    RADIO->PCNF0 = (8 << RADIO_PCNF0_LFLEN_Shft)  |
                   (0 << RADIO_PCNF0_S0LEN_Shft)  |
                   (0 << RADIO_PCNF0_S1LEN_Shft)  |
                   (RADIO_PCNF0_S1INCL_Automatic) |
                   (RADIO_PCNF0_PLEN_8bit);

    /* dynamic payload size determined by LENGTH,
     * no need for STATLEN
     */
    RADIO->PCNF1 = (8 << RADIO_PCNF1_MAXLEN_Shft)   |
                   (0 << RADIO_PCNF1_STATLEN_Shft)  |
                   (3 << RADIO_PCNF1_BALEN_Shft)    |
                   (RADIO_PCNF1_ENDIAN_Little)      |
                   (RADIO_PCNF1_WHITEEN_Enabled);
    
    /* initial whitening value */
    RADIO->DATAWHITEIV = 0x69;

    /* configure CRC */
    RADIO->CRCCNF = (2 << RADIO_CRCCNF_LEN_Shft) |
                    (RADIO_CRCCNF_SKIPADDR_Skip);
    RADIO->CRCPOLY = 0x001685F1;
    RADIO->CRCINIT = 0x000656E9;

    /* 2402 MHz, 0dBm */
    RADIO->FREQUENCY = 2;
    RADIO->TXPOWER   = 0;

    /* configure on-air address.
     * = [PREFIX byte] + [BALEN bytes of BASE]
     */
    RADIO->BASE0   = 0x00440967;
    RADIO->BASE1   = 0x00764635;
    RADIO->PREFIX0 = 0x2A5F;

    /* on-air address using BASE0 and first 2 bytes of
     * PREFIX0 translates to a logical address of 0.
     * see p.601 table 35
     */
    RADIO->TXADDRESS   = 0;
    RADIO->RXADDRESSES = RADIO_RXADDRESSES_ADDR1_Enabled;

    /* shortcuts */
    RADIO->SHORTS = RADIO_SHORTS_END_DISABLE_ |
                    RADIO_SHORTS_RXREADY_START_;
    
    RADIO->INTENSET = RADIO_INTENSET_TXREADY_Set
                    | RADIO_INTENSET_DISABLED_Set;

    NVIC->ISER[NVIC_RADIO_IRQ / 32] = (1 << (NVIC_RADIO_IRQ % 32));

}

static void comp_setup(void) {

    /* we will compare vddh/5 to decreasing fractions of the
     * 1.2v internal reference until we cross vddh (vbat).
     */
    COMP->MODE   = COMP_MODE_MAIN_SE | COMP_MODE_SP_Normal;
    COMP->PSEL   = COMP_PSEL_PSEL_VddhDiv5;
    COMP->REFSEL = COMP_REFSEL_REFSEL_Int1v2;

    COMP->SHORTS = COMP_SHORTS_READY_SAMPLE_Enabled
                 | COMP_SHORTS_READY_STOP_Enabled;

    COMP->INTENSET = COMP_INTENSET_READY_Set;
    NVIC->ISER[NVIC_COMP_LPCOMP_IRQ / 32] = (1 << (NVIC_COMP_LPCOMP_IRQ % 32));

}

static void comp_start(uint8_t ladder_pos) {

    COMP->TH = (ladder_pos << COMP_TH_THUP_Shft)
             | (ladder_pos << COMP_TH_THDOWN_Shft);

    COMP->EVENTS_READY = 0;
    COMP->TASKS_START  = 1;

}

static void async_get_vbat(void) {

    if (comp_ctx.active) return;

    comp_ctx.active = 1;
    comp_ctx.ladder = 44;
    
    COMP->ENABLE = COMP_ENABLE_ENABLE_Enabled;
    comp_start(comp_ctx.ladder);

}

static void async_paw_motion_burst(void) {

    if (spim_ctx.active) return;

    /* start paw3395 motion burst by sending motion
     * burst address
     */
    static uint8_t addr = PAW3395_MOTION_BURST;
    spim_ctx.active = 1;
    spim_ctx.ready  = 0;
    spim_ctx.state  = SPIM_STATE_TX;

    /* pull NCS low to select paw */
    P0->OUTCLR = (1 << NCS_PIN);

    SPIM0->EVENTS_END = 0;
    SPIM0->INTENSET   = SPIM_INTENSET_END_Set;

    SPIM0->TXD.PTR     = (uint32_t) &addr;
    SPIM0->TXD.MAXCNT  = 1;
    SPIM0->RXD.PTR     = (uint32_t) NULL;
    SPIM0->RXD.MAXCNT  = 0;

    SPIM0->TASKS_START = 1;

}

static void fill_mouse_pkt(void) {

    QDEC->TASKS_RDCLRACC = 1;
    mouse_pkt.btn_vbat = (l_click << 0) | (r_click << 1) | (vbat << 2);
    mouse_pkt.dx       = (int16_t) ((paw_data[3] << 8) | (paw_data[2] << 0));
    mouse_pkt.dy       = (int16_t) ((paw_data[5] << 8) | (paw_data[4] << 0));
    mouse_pkt.wheel    = (int8_t) QDEC->ACCREAD;
    op_mode = paw_data[0] & PAW3395_MOTION_OP_MODE_Msk;

}

static void enter_sleep(void) {

    /* disable irqs */
    TIMER0->INTENCLR = 0xFFFFFFFF;
    TIMER1->INTENCLR = 0xFFFFFFFF;
    GPIOTE->INTENCLR = 0xFFFFFFFF;
    SPIM0->INTENCLR  = 0xFFFFFFFF;
    COMP->INTENCLR   = 0xFFFFFFFF;
    RADIO->INTENCLR  = 0xFFFFFFFF;
    NVIC->ICER[0]    = 0xFFFFFFFF;
    NVIC->ICER[1]    = 0xFFFFFFFF;

    /* low-power mode */
    POWER->TASKS_LOWPWR = 1;

    /* disable peripherals (radio/comp already disabled) */
    TIMER0->TASKS_SHUTDOWN  = 1;
    TIMER0->TASKS_CLEAR     = 1;
    TIMER1->TASKS_SHUTDOWN  = 1;    /* errata no.78 */  
    TIMER1->TASKS_CLEAR     = 1;
    QDEC->TASKS_STOP        = 1;
    while (!(QDEC->EVENTS_STOPPED));
    QDEC->EVENTS_STOPPED    = 0;
    QDEC->ENABLE            = QDEC_ENABLE_ENABLE_Disabled;
    SPIM0->ENABLE           = SPIM_ENABLE_ENABLE_Disabled;
    CLOCK->TASKS_HFCLKSTOP  = 1;

    /* set all GPIOs back to reset values */
    uint8_t pins[] = {L_NO_PIN, L_NC_PIN, R_NO_PIN, R_NC_PIN,
                      NCS_PIN,  MISO_PIN, MOSI_PIN, SCK_PIN,
                      ENC_A_PIN, ENC_B_PIN};
    for (uint8_t i = 0; i < ARR_SIZE(pins); i++) {
        P0->PIN_CNF[pins[i]] = GPIO_PIN_CNF_DIR_Input
                             | GPIO_PIN_CNF_INPUT_Disconnect
                             | GPIO_PIN_CNF_PULL_Disabled
                             | GPIO_PIN_CNF_DRIVE_S0S1
                             | GPIO_PIN_CNF_SENSE_Disabled;

        GPIOTE->CONFIG[i & 0b111]    = 0;
        GPIOTE->EVENTS_IN[i & 0b111] = 0;
    }

    /* hold NCS high */
    P0->PIN_CNF[NCS_PIN] = GPIO_PIN_CNF_DIR_Input
                         | GPIO_PIN_CNF_INPUT_Disconnect
                         | GPIO_PIN_CNF_PULL_Pullup
                         | GPIO_PIN_CNF_DRIVE_S0S1
                         | GPIO_PIN_CNF_SENSE_Disabled;

    /* configure wakeup on GPIOTE PORT event */
    P0->PIN_CNF[MOTION_PIN] = GPIO_PIN_CNF_DIR_Input
                            | GPIO_PIN_CNF_INPUT_Connect
                            | GPIO_PIN_CNF_PULL_Disabled
                            | GPIO_PIN_CNF_DRIVE_S0S1
                            | GPIO_PIN_CNF_SENSE_Low;
    GPIOTE->EVENTS_PORT = 0;
    GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Set;
    NVIC->ISER[NVIC_GPIOTE_IRQ / 32] = (1 << (NVIC_GPIOTE_IRQ % 32));

}

static void exit_sleep(void) {

    /* disable GPIOTE PORT event */
    GPIOTE->INTENCLR = 0xFFFFFFFF;
    P0->PIN_CNF[MOTION_PIN] = GPIO_PIN_CNF_INPUT_Disconnect;
    GPIOTE->EVENTS_PORT = 0;

    /* blink LED to show we still have power */
    P0->DIRSET = (1 << LED_PIN);
    delay_us(TIMER0, 100000);
    P0->DIRCLR = (1 << LED_PIN);

    /* re-enable peripherals */
    power_setup();
    clock_setup();
    timer_setup();
    gpio_setup();
    gpiote_setup();
    qdec_setup();
    spi_setup();
    comp_setup();
    radio_setup();

    /* restart isr timer */
    TIMER1->TASKS_START = 1;

}

int main(void) {

    power_setup();
    clock_setup();
    timer_setup();
    gpio_setup();
    gpiote_setup();
    qdec_setup();
    spi_setup();
    comp_setup();
    radio_setup();

    /* recommended power-up sequence from datasheet */
    paw_init();

    /* start 1KHz timer isr, later synchronized w/ dongle */
    TIMER1->TASKS_START = 1;

    for (;;) {
        __asm__("wfi");
    }

}

void timer1_isr(void) {

    /* TX slot reached -- start TX->RX sequence */
    if (TIMER1->EVENTS_COMPARE[0]) {
        TIMER1->EVENTS_COMPARE[0] = 0;

        RADIO->PACKETPTR = (uint32_t) &mouse_pkt;
        RADIO->TASKS_TXEN = 1;

        async_paw_motion_burst();
        radio_ctx.state = RADIO_STATE_TXRU;

    }

    /* RX timeout -- enter RXDISABLE */
    if (TIMER1->EVENTS_COMPARE[1]) {
        TIMER1->EVENTS_COMPARE[1] = 0;
        
        TIMER1->TASKS_STOP  = 1;
        TIMER1->TASKS_CLEAR = 1;
        TIMER1->CC[1]       = 0xFFFFFFFF;

        RADIO->TASKS_DISABLE = 1;
        radio_ctx.state = RADIO_STATE_RXTO_RXDISABLE;

    }

}

void radio_isr(void) {

    /* TX ramp-up complete */
    if (RADIO->EVENTS_TXREADY) {
        RADIO->EVENTS_TXREADY = 0;

        if (spim_ctx.ready) {
            fill_mouse_pkt();
        }

        RADIO->TASKS_START = 1;
        radio_ctx.state = RADIO_STATE_TX;

    }

    /* prev xfer complete */
    if (RADIO->EVENTS_DISABLED) {
        RADIO->EVENTS_DISABLED = 0;

        switch (radio_ctx.state) {
        
            /* packet sent, start RX and RX timeout
             */
            case RADIO_STATE_TX:

                if (op_mode == PAW3395_MOTION_OP_MODE_Rest3) {
                    enter_sleep();
                    return;
                }

                RADIO->PACKETPTR  = (uint32_t) &dongle_pkt;
                RADIO->TASKS_RXEN = 1;

                TIMER1->EVENTS_COMPARE[1] = 0;
                TIMER1->CC[1] = RX_TIMEOUT_US;
                TIMER1->TASKS_START = 1;

                radio_ctx.state = RADIO_STATE_RX;
                break;

            /* disable task from RX timeout completed, start TX timer 
             */
            case RADIO_STATE_RXTO_RXDISABLE:

                TIMER1->TASKS_START = 1;
                radio_ctx.state = RADIO_STATE_DISABLED;
                break;

            /* packet recv'd, evaluate pkt and start TX timer 
             */
            case RADIO_STATE_RX:
                
                TIMER1->TASKS_STOP  = 1;
                TIMER1->TASKS_CLEAR = 1;
                TIMER1->EVENTS_COMPARE[1] = 0;
                TIMER1->CC[1] = 0xFFFFFFFF;

                radio_ctx.state = RADIO_STATE_DISABLED;

                if (!(RADIO->CRCSTATUS)) {
                    TIMER1->TASKS_START = 1;
                    return;
                }

                if (dongle_pkt.cc < RX_TIMEOUT_US + 50) {
                    dongle_pkt.cc = RX_TIMEOUT_US + 50;
                }

                TIMER1->CC[0] = dongle_pkt.cc;
                TIMER1->TASKS_START = 1;

                if (curr_dpi != dongle_pkt.dpi) {
                    paw_set_dpi(dongle_pkt.dpi);
                    curr_dpi = dongle_pkt.dpi;
                }

                elapsed_us += dongle_pkt.cc;
                if (elapsed_us > VBAT_INTERVAL) {
                    elapsed_us = 0;
                    async_get_vbat();
                }
                break;
            
            default:
                break;

        }

    }
    
}

void spi0_spim0_spis0_twi0_twim0_twis0_isr(void) {

    if (SPIM0->EVENTS_END) {
        SPIM0->EVENTS_END = 0;

        if (spim_ctx.state == SPIM_STATE_TX) {

            /* motion burst addr sent, start receiving after t_srad (2us)
             */
            TIMER0->CC[0]       = 2;
            TIMER0->INTENSET    = TIMER_INTENSET_COMPARE0_Set;
            TIMER0->TASKS_START = 1;
            spim_ctx.state      = SPIM_STATE_RX;

        }
        else { /* burst received, data ready */

            P0->OUTSET = (1 << NCS_PIN);
            SPIM0->INTENCLR = SPIM_INTENCLR_End_Clear;
            spim_ctx.active = 0;
            spim_ctx.ready  = 1;

        }

    }

}

void timer0_isr(void) {

    /* t_srad elapsed, receive burst data */
    if (TIMER0->EVENTS_COMPARE[0]) {
        TIMER0->EVENTS_COMPARE[0] = 0;

        TIMER0->TASKS_STOP  = 1;
        TIMER0->TASKS_CLEAR = 1;
        TIMER0->INTENCLR = TIMER_INTENCLR_COMPARE0_Clear;

        SPIM0->TXD.PTR    = (uint32_t) NULL;
        SPIM0->TXD.MAXCNT = 0;
        SPIM0->RXD.PTR    = (uint32_t) paw_data;
        SPIM0->RXD.MAXCNT = BURST_SIZE;

        SPIM0->TASKS_START = 1;

    }

}

void gpiote_isr(void) {

    /* SPDT 2-pin debounce (SR-latch emulation) */

    /* switch closure:
     * disable further events on L_NO (ignore bouncing)
     * enable falling event on L_NC   (wait for switch re-open)
     */
    if (GPIOTE->EVENTS_IN[CH_L_NO]) {
        GPIOTE->EVENTS_IN[CH_L_NO] = 0;

        GPIOTE->CONFIG[CH_L_NO] = 0;
        GPIOTE->CONFIG[CH_L_NC] = GPIOTE_CONFIG_MODE_Event
                                | GPIOTE_CONFIG_POLARITY_HiToLo
                                | (L_NC_PIN << GPIOTE_CONFIG_PSEL_Shft);

        GPIOTE->EVENTS_IN[CH_L_NC] = 0;
        
        l_click = 1;

    }
    
    /* switch re-open: 
     * disable further events on L_NC (ignore bouncing)
     * enable falling event on L_NO   (wait for switch closure)
     */
    if (GPIOTE->EVENTS_IN[CH_L_NC]) {
        GPIOTE->EVENTS_IN[CH_L_NC] = 0;

        GPIOTE->CONFIG[CH_L_NC] = 0;
        GPIOTE->CONFIG[CH_L_NO] = GPIOTE_CONFIG_MODE_Event
                                | GPIOTE_CONFIG_POLARITY_HiToLo
                                | (L_NO_PIN << GPIOTE_CONFIG_PSEL_Shft);
        
        GPIOTE->EVENTS_IN[CH_L_NO] = 0;
        
        l_click = 0;

    }
    
    if (GPIOTE->EVENTS_IN[CH_R_NO]) {
        GPIOTE->EVENTS_IN[CH_R_NO] = 0;

        GPIOTE->CONFIG[CH_R_NO] = 0;
        GPIOTE->CONFIG[CH_R_NC] = GPIOTE_CONFIG_MODE_Event
                                | GPIOTE_CONFIG_POLARITY_HiToLo
                                | (R_NC_PIN << GPIOTE_CONFIG_PSEL_Shft);
        
        GPIOTE->EVENTS_IN[CH_R_NC] = 0;
        
        r_click = 1;

    }
    
    if (GPIOTE->EVENTS_IN[CH_R_NC]) {
        GPIOTE->EVENTS_IN[CH_R_NC] = 0;

        GPIOTE->CONFIG[CH_R_NC] = 0;
        GPIOTE->CONFIG[CH_R_NO] = GPIOTE_CONFIG_MODE_Event
                                | GPIOTE_CONFIG_POLARITY_HiToLo
                                | (R_NO_PIN << GPIOTE_CONFIG_PSEL_Shft);
        
        GPIOTE->EVENTS_IN[CH_R_NO] = 0;
        
        r_click = 0;

    }

    if (GPIOTE->EVENTS_PORT) {
        GPIOTE->EVENTS_PORT = 0;
        exit_sleep();
    }

}

void comp_lpcomp_isr(void) {

    /* prev voltage comparison finished,
     * stop if vbat was found or we've gone below 3.0V, 
     * otherwise, decrement the ladder and try again 
     */
    if (COMP->EVENTS_READY) {
        COMP->EVENTS_READY = 0;

        uint32_t res = COMP->RESULT;
        if (res || comp_ctx.ladder < 31) {

            COMP->ENABLE    = COMP_ENABLE_ENABLE_Disabled;
            comp_ctx.active = 0;

            vbat = comp_ctx.ladder;
            return;
        }
        
        comp_ctx.ladder--;
        comp_start(comp_ctx.ladder);

    }

}

