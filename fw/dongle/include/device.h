/**********************************************************************************
 ** file        : device.h
 ** description : structs and macros for core/device peripheral access
 ** 
 ** device      : nRF52840
 **
 **********************************************************************************/

#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

#define IO32 volatile uint32_t
#define IO16 volatile uint16_t
#define IO8  volatile uint8_t

/* --- PERIPHERAL STRUCTS ------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */

typedef struct {
    IO32 POWER;
    IO32 POWERSET;
    IO32 POWERCLR;
    IO32 RESERVED;
} POWER_RAM_T;

typedef struct {
    IO32 RESERVED[30];
    IO32 TASKS_CONSTLAT;
    IO32 TASKS_LOWPWR;
    IO32 RESERVED1[34];
    IO32 EVENTS_POFWARN;
    IO32 RESERVED2[2];
    IO32 EVENTS_SLEEPENTER;
    IO32 EVENTS_SLEEPEXIT;
    IO32 EVENTS_USBDETECTED;
    IO32 EVENTS_USBREMOVED;
    IO32 EVENTS_USBPWRRDY;
    IO32 RESERVED3[119];
    IO32 INTENSET;
    IO32 INTENCLR;
    IO32 RESERVED4[61];
    IO32 RESETREAS;
    IO32 RESERVED5[9];
    IO32 RAMSTATUS;
    IO32 RESERVED6[3];
    IO32 USBREGSTATUS;
    IO32 RESERVED7[49];
    IO32 SYSTEMOFF;
    IO32 RESERVED8[3];
    IO32 POFCON;
    IO32 RESERVED9[2];
    IO32 GPREGRET;
    IO32 GPREGRET2;
    IO32 RESERVED10[21];
    IO32 DCDCEN;
    IO32 RESERVED11;
    IO32 DCDCEN0;
    IO32 RESERVED12[47];
    IO32 MAINREGSTATUS;
    IO32 RESERVED13[175];
    POWER_RAM_T RAM[9];
} POWER_T;

typedef struct {
    IO32 TASKS_HFCLKSTART;
    IO32 TASKS_HFCLKSTOP;
    IO32 TASKS_LFCLKSTART;
    IO32 TASKS_LFCLKSTOP;
    IO32 TASKS_CAL;
    IO32 TASKS_CTSTART;
    IO32 TASKS_CTSTOP;
    IO32 RESERVED[57];
    IO32 EVENTS_HFCLKSTARTED;
    IO32 EVENTS_LFCLKSTARTED;
    IO32 RESERVED1;
    IO32 EVENTS_DONE;
    IO32 EVENTS_CTTO;
    IO32 RESERVED2[5];
    IO32 EVENTS_CTSTARTED;
    IO32 EVENTS_CTSTOPPED;
    IO32 RESERVED3[117];
    IO32 INTENSET;
    IO32 INTENCLR;
    IO32 RESERVED4[63];
    IO32 HFCLKRUN;
    IO32 HFCLKSTAT;
    IO32 RESERVED5;
    IO32 LFCLKRUN;
    IO32 LFCLKSTAT;
    IO32 LFCLKSRCCOPY;
    IO32 RESERVED6[62];
    IO32 LFCLKSRC;
    IO32 RESERVED7[3];
    IO32 HFXODEBOUNCE;
    IO32 RESERVED8[3];
    IO32 CTIV;
    IO32 RESERVED9[8];
    IO32 TRACECONFIG;
    IO32 RESERVED10[21];
    IO32 LFRCMODE;
} CLOCK_T;

typedef struct {
    IO32 TASKS_TXEN;
    IO32 TASKS_RXEN;
    IO32 TASKS_START;
    IO32 TASKS_STOP;
    IO32 TASKS_DISABLE;
    IO32 TASKS_RSSISTART;
    IO32 TASKS_RSSISTOP;
    IO32 TASKS_BCSTART;
    IO32 TASKS_BCSTOP;
    IO32 TASKS_EDSTART;
    IO32 TASKS_EDSTOP;
    IO32 TASKS_CCASTART;
    IO32 TASKS_CCASTOP;
    IO32 RESERVED[51];
    IO32 EVENTS_READY;
    IO32 EVENTS_ADDRESS;
    IO32 EVENTS_PAYLOAD;
    IO32 EVENTS_END;
    IO32 EVENTS_DISABLED;
    IO32 EVENTS_DEVMATCH;
    IO32 EVENTS_DEVMISS;
    IO32 EVENTS_RSSIEND;
    IO32 RESERVED1[2];
    IO32 EVENTS_BCMATCH;
    IO32 RESERVED2;
    IO32 EVENTS_CRCOK;
    IO32 EVENTS_CRCERROR;
    IO32 EVENTS_FRAMESTART;
    IO32 EVENTS_EDEND;
    IO32 EVENTS_EDSTOPPED;
    IO32 EVENTS_CCAIDLE;
    IO32 EVENTS_CCABUSY;
    IO32 EVENTS_CCASTOPPED;
    IO32 EVENTS_RATEBOOST;
    IO32 EVENTS_TXREADY;
    IO32 EVENTS_RXREADY;
    IO32 EVENTS_MHRMATCH;
    IO32 RESERVED3[2];
    IO32 EVENTS_SYNC;
    IO32 EVENTS_PHYEND;
    IO32 RESERVED4[36];
    IO32 SHORTS;
    IO32 RESERVED5[64];
    IO32 INTENSET;
    IO32 INTENCLR;
    IO32 RESERVED6[61];
    IO32 CRCSTATUS;
    IO32 RESERVED7;
    IO32 RXMATCH;
    IO32 RXCRC;
    IO32 DAI;
    IO32 PDUSTAT;
    IO32 RESERVED8[59];
    IO32 PACKETPTR;
    IO32 FREQUENCY;
    IO32 TXPOWER;
    IO32 MODE;
    IO32 PCNF0;
    IO32 PCNF1;
    IO32 BASE0;
    IO32 BASE1;
    IO32 PREFIX0;
    IO32 PREFIX1;
    IO32 TXADDRESS;
    IO32 RXADDRESSES;
    IO32 CRCCNF;
    IO32 CRCPOLY;
    IO32 CRCINIT;
    IO32 RESERVED9;
    IO32 TIFS;
    IO32 RSSISAMPLE;
    IO32 RESERVED10;
    IO32 STATE;
    IO32 DATAWHITEIV;
    IO32 RESERVED11[2];
    IO32 BCC;
    IO32 RESERVED12[39];
    IO32 DAB[8];
    IO32 DAP[8];
    IO32 DACNF;
    IO32 MHRMATCHCONF;
    IO32 MHRMATCHMAS;
    IO32 RESERVED13;
    IO32 MODECNF0;
    IO32 RESERVED14[3];
    IO32 SFD;
    IO32 EDCNT;
    IO32 EDSAMPLE;
    IO32 CCACTRL;
    IO32 RESERVED15[611];
    IO32 POWER;
} RADIO_T;

typedef struct {
    IO32 TASKS_START;
    IO32 TASKS_STOP;
    IO32 TASKS_COUNT;
    IO32 TASKS_CLEAR;
    IO32 TASKS_SHUTDOWN;
    IO32 RESERVED[11];
    IO32 TASKS_CAPTURE[6];
    IO32 RESERVED1[58];
    IO32 EVENTS_COMPARE[6];
    IO32 RESERVED2[42];
    IO32 SHORTS;
    IO32 RESERVED3[64];
    IO32 INTENSET;
    IO32 INTENCLR;
    IO32 RESERVED4[126];
    IO32 MODE;
    IO32 BITMODE;
    IO32 RESERVED5;
    IO32 PRESCALER;
    IO32 RESERVED6[11];
    IO32 CC[6];
} TIMER_T;

typedef struct {
    IO32 EPIN[8];
    IO32 RESERVED;
    IO32 EPOUT[8];
} USBD_HALTED_T;

typedef struct {
    IO32 EPOUT[8];
    IO32 ISOOUT;
} USBD_SIZE_T;

typedef struct {
    IO32 PTR;
    IO32 MAXCNT;
    IO32 AMOUNT;
    IO32 RESERVED[2];
} USBD_EPIN_T;

typedef struct {
    IO32 PTR;
    IO32 MAXCNT;
    IO32 AMOUNT;
} USBD_ISOIN_T;

typedef struct {
    IO32 PTR;
    IO32 MAXCNT;
    IO32 AMOUNT;
    IO32 RESERVED[2];
} USBD_EPOUT_T;

typedef struct {
    IO32 PTR;
    IO32 MAXCNT;
    IO32 AMOUNT;
} USBD_ISOOUT_T;

typedef struct {
    IO32 RESERVED;
    IO32 TASKS_STARTEPIN[8];
    IO32 TASKS_STARTISOIN;
    IO32 TASKS_STARTEPOUT[8];
    IO32 TASKS_STARTISOOUT;
    IO32 TASKS_EP0RCVOUT;
    IO32 TASKS_EP0STATUS;
    IO32 TASKS_EP0STALL;
    IO32 TASKS_DPDMDRIVE;
    IO32 TASKS_DPDMNODRIVE;
    IO32 RESERVED1[40];
    IO32 EVENTS_USBRESET;
    IO32 EVENTS_STARTED;
    IO32 EVENTS_ENDEPIN[8];
    IO32 EVENTS_EP0DATADONE;
    IO32 EVENTS_ENDISOIN;
    IO32 EVENTS_ENDEPOUT[8];
    IO32 EVENTS_ENDISOOUT;
    IO32 EVENTS_SOF;
    IO32 EVENTS_USBEVENT;
    IO32 EVENTS_EP0SETUP;
    IO32 EVENTS_EPDATA;
    IO32 RESERVED2[39];
    IO32 SHORTS;
    IO32 RESERVED3[63];
    IO32 INTEN;
    IO32 INTENSET;
    IO32 INTENCLR;
    IO32 RESERVED4[61];
    IO32 EVENTCAUSE;
    IO32 RESERVED5[7];
    USBD_HALTED_T HALTED;
    IO32 RESERVED6;
    IO32 EPSTATUS;
    IO32 EPDATASTATUS;
    IO32 USBADDR;
    IO32 RESERVED7[3];
    IO32 BMREQUESTTYPE;
    IO32 BREQUEST;
    IO32 WVALUEL;
    IO32 WVALUEH;
    IO32 WINDEXL;
    IO32 WINDEXH;
    IO32 WLENGTHL;
    IO32 WLENGTHH;
    USBD_SIZE_T SIZE;
    IO32 RESERVED8[15];
    IO32 ENABLE;
    IO32 USBPULLUP;
    IO32 DPDMVALUE;
    IO32 DTOGGLE;
    IO32 EPINEN;
    IO32 EPOUTEN;
    IO32 EPSTALL;
    IO32 ISOSPLIT;
    IO32 FRAMECNTR;
    IO32 RESERVED9[2];
    IO32 LOWPOWER;
    IO32 ISOINCONFIG;
    IO32 RESERVED10[51];
    USBD_EPIN_T EPIN[8];
    USBD_ISOIN_T ISOIN;
    IO32 RESERVED11[21];
    USBD_EPOUT_T EPOUT[8];
    USBD_ISOOUT_T ISOOUT;
} USBD_T;

typedef struct {
    IO32 RESERVED[321];
    IO32 OUT;
    IO32 OUTSET;
    IO32 OUTCLR;
    IO32 IN;
    IO32 DIR;
    IO32 DIRSET;
    IO32 DIRCLR;
    IO32 LATCH;
    IO32 DETECTMODE;
    IO32 RESERVED1[118];
    IO32 PIN_CNF[32];
} GPIO_T;

typedef struct {
    IO32 ISER[8];
    IO32 RESERVED0[24];
    IO32 ICER[8];
    IO32 RESERVED1[24];
    IO32 ISPR[8];
    IO32 RESERVED2[24];
    IO32 ICPR[8];
    IO32 RESERVED3[24];
    IO32 IABR[8];
    IO32 RESERVED4[56];
    IO8  IPR[240];
    IO32 RESERVED5[644];
    IO32 STIR;
} NVIC_T;

/* --- BITFIELDS --------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* --- SHIFT VALUES -------------------------------------------------------- */

#define RADIO_PCNF0_LFLEN_Shft          0U
#define RADIO_PCNF0_S0LEN_Shft          8U
#define RADIO_PCNF0_S1LEN_Shft          16U
#define RADIO_PCNF0_S1INCL_Shft         20U
#define RADIO_PCNF0_PLEN_Shft           24U
#define RADIO_PCNF1_MAXLEN_Shft         0U
#define RADIO_PCNF1_STATLEN_Shft        8U
#define RADIO_PCNF1_BALEN_Shft          16U
#define RADIO_CRCCNF_LEN_Shft           0U
#define RADIO_CRCCNF_SKIPADDR_Shft      8U
#define RADIO_MODECNF0_RU_Shft          0U
#define RADIO_MODECNF0_DTX_Shft         8U

#define USBD_INTEN_ENDEPIN0_Shft        2U
#define USBD_INTEN_ENDEPOUT0_Shft       12U
#define USBD_INTEN_EPDATA_Shft          24U

#define GPIO_PIN_CNF_DIR_Shft           0U
#define GPIO_PIN_CNF_INPUT_Shft         1U
#define GPIO_PIN_CNF_PULL_Shft          2U
#define GPIO_PIN_CNF_DRIVE_Shft         8U
#define GPIO_PIN_CNF_SENSE_Shft        16U

/* --- BITMASKS ------------------------------------------------------------ */

/* --- BITFIELD VALUES --------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

/* --- CLOCK --------------------------------------------------------------- */

/* --- RADIO --------------------------------------------------------------- */

#define RADIO_SHORTS_READY_START_                           (1 << 0)
#define RADIO_SHORTS_END_DISABLE_                           (1 << 1)
#define RADIO_SHORTS_DISABLED_TXEN_                         (1 << 2)
#define RADIO_SHORTS_DISABLED_RXEN_                         (1 << 3)
#define RADIO_SHORTS_END_START_                             (1 << 5)
#define RADIO_SHORTS_RXREADY_START_                         (1 << 19)

#define RADIO_INTENSET_END_Set                              (1 << 3)
#define RADIO_INTENSET_DISABLED_Set                         (1 << 4)

#define RADIO_MODE_MODE_Nrf_1Mbit                           (0b0000 << 0)
#define RADIO_MODE_MODE_Nrf_2Mbit                           (0b0001 << 0)

#define RADIO_PCNF0_S1INCL_Automatic                        (0    << RADIO_PCNF0_S1INCL_Shft)
#define RADIO_PCNF0_S1INCL_Include                          (1    << RADIO_PCNF0_S1INCL_Shft)
#define RADIO_PCNF0_PLEN_8bit                               (0b00 << RADIO_PCNF0_PLEN_Shft)
#define RADIO_PCNF0_PLEN_16bit                              (0b01 << RADIO_PCNF0_PLEN_Shft)

#define RADIO_PCNF1_ENDIAN_Little                           (0 << 24)
#define RADIO_PCNF1_ENDIAN_Big                              (1 << 24)
#define RADIO_PCNF1_WHITEEN_Disabled                        (0 << 25)
#define RADIO_PCNF1_WHITEEN_Enabled                         (1 << 25)

#define RADIO_RXADDRESSES_ADDR0_Enabled                     (1 << 0)
#define RADIO_RXADDRESSES_ADDR1_Enabled                     (1 << 1)

#define RADIO_CRCCNF_SKIPADDR_Skip                          (0b01 << RADIO_CRCCNF_SKIPADDR_Shft)

#define RADIO_MODECNF0_RU_Default                           (0 << RADIO_MODECNF0_RU_Shft)
#define RADIO_MODECNF0_RU_Fast                              (1 << RADIO_MODECNF0_RU_Shft)
#define RADIO_MODECNF0_DTX_Center                           (0b10 << RADIO_MODECNF0_DTX_Shft)

/* --- TIMER --------------------------------------------------------------- */

#define TIMER_SHORTS_COMPARE0_CLEAR_Enabled                 (1 << 0)
#define TIMER_SHORTS_COMPARE0_STOP_Enabled                  (1 << 8)

#define TIMER_INTENSET_COMPARE0_Set                         (1 << 16)
#define TIMER_INTENCLR_COMPARE0_Clr                         (1 << 16)

#define TIMER_MODE_MODE_Timer                               (0 << 0)
#define TIMER_MODE_MODE_Counter                             (1 << 0)

#define TIMER_BITMODE_BITMODE_16Bit                         (0b00 << 0)
#define TIMER_BITMODE_BITMODE_08Bit                         (0b01 << 0)
#define TIMER_BITMODE_BITMODE_24Bit                         (0b10 << 0)
#define TIMER_BITMODE_BITMODE_32Bit                         (0b11 << 0)

/* --- USBD ---------------------------------------------------------------- */

#define USBD_EVENTCAUSE_READY_                              (1 << 11)

#define USBD_INTEN_USBRESET_                                (1 << 0)
#define USBD_INTEN_ENDEPIN0_                                (1 << 2)
#define USBD_INTEN_EP0DATADONE_                             (1 << 10)
#define USBD_INTEN_ENDEPOUT0_                               (1 << 12)
#define USBD_INTEN_USBEVENT_                                (1 << 22)
#define USBD_INTEN_EP0SETUP_                                (1 << 23)
#define USBD_INTEN_EPDATA_                                  (1 << 24)
#define USBD_INTEN_ENDEP_                                   ((0xFF << USBD_INTEN_ENDEPIN0_Shft) | (0xFF << USBD_INTEN_ENDEPOUT0_Shft))

/* --- GPIO ---------------------------------------------------------------- */

#define GPIO0                                               (1 << 0)
#define GPIO1                                               (1 << 1)
#define GPIO2                                               (1 << 2)
#define GPIO3                                               (1 << 3)
#define GPIO4                                               (1 << 4)
#define GPIO5                                               (1 << 5)
#define GPIO6                                               (1 << 6)
#define GPIO7                                               (1 << 7)
#define GPIO8                                               (1 << 8)
#define GPIO9                                               (1 << 9)
#define GPIO10                                              (1 << 10)
#define GPIO11                                              (1 << 11)
#define GPIO12                                              (1 << 12)
#define GPIO13                                              (1 << 13)
#define GPIO14                                              (1 << 14)
#define GPIO15                                              (1 << 15)

#define GPIO_PIN_CNF_DIR_Input                              (0 << GPIO_PIN_CNF_DIR_Shft)
#define GPIO_PIN_CNF_DIR_Output                             (1 << GPIO_PIN_CNF_DIR_Shft)
#define GPIO_PIN_CNF_INPUT_Connect                          (0 << GPIO_PIN_CNF_INPUT_Shft)
#define GPIO_PIN_CNF_INPUT_Disconnect                       (1 << GPIO_PIN_CNF_INPUT_Shft)
#define GPIO_PIN_CNF_PULL_Disabled                          (0b00 << GPIO_PIN_CNF_PULL_Shft)
#define GPIO_PIN_CNF_PULL_Pulldown                          (0b01 << GPIO_PIN_CNF_PULL_Shft)
#define GPIO_PIN_CNF_PULL_Pullup                            (0b11 << GPIO_PIN_CNF_PULL_Shft)
#define GPIO_PIN_CNF_DRIVE_S0S1                             (0b000 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_H0S1                             (0b001 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_S0H1                             (0b010 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_H0H1                             (0b011 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_D0S1                             (0b100 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_D0H1                             (0b101 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_S0D1                             (0b110 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_DRIVE_H0D1                             (0b111 << GPIO_PIN_CNF_DRIVE_Shft)
#define GPIO_PIN_CNF_SENSE_Disabled                         (0b00 << GPIO_PIN_CNF_SENSE_Shft)
#define GPIO_PIN_CNF_SENSE_High                             (0b10 << GPIO_PIN_CNF_SENSE_Shft)
#define GPIO_PIN_CNF_SENSE_Low                              (0b11 << GPIO_PIN_CNF_SENSE_Shft)

/* --- NVIC IRQ Numbers -------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

#define NVIC_CLOCK_POWER_IRQ                        0
#define NVIC_RADIO_IRQ                              1
#define NVIC_UART0_UARTE0_IRQ                       2
#define NVIC_SPI0_SPIM0_SPIS0_TWI0_TWIM0_TWIS0_IRQ  3
#define NVIC_SPI1_SPIM1_SPIS1_TWI1_TWIM1_TWIS1_IRQ  4
#define NVIC_NFCT_IRQ                               5
#define NVIC_GPIOTE_IRQ                             6
#define NVIC_SAADC_IRQ                              7
#define NVIC_TIMER0_IRQ                             8
#define NVIC_TIMER1_IRQ                             9
#define NVIC_TIMER2_IRQ                             10
#define NVIC_RTC0_IRQ                               11
#define NVIC_TEMP_IRQ                               12
#define NVIC_RNG_IRQ                                13
#define NVIC_ECB_IRQ                                14
#define NVIC_AAR_CCM_IRQ                            15
#define NVIC_WDT_IRQ                                16
#define NVIC_RTC1_IRQ                               17
#define NVIC_QDEC_IRQ                               18
#define NVIC_COMP_LPCOMP_IRQ                        19
#define NVIC_EGU0_SWI0_IRQ                          20
#define NVIC_EGU1_SWI1_IRQ                          21
#define NVIC_EGU2_SWI2_IRQ                          22
#define NVIC_EGU3_SWI3_IRQ                          23
#define NVIC_EGU4_SWI4_IRQ                          24
#define NVIC_EGU5_SWI5_IRQ                          25
#define NVIC_TIMER3_IRQ                             26
#define NVIC_TIMER4_IRQ                             27
#define NVIC_PWM0_IRQ                               28
#define NVIC_PDM_IRQ                                29
#define NVIC_MWU_IRQ                                32
#define NVIC_PWM1_IRQ                               33
#define NVIC_PWM2_IRQ                               34
#define NVIC_SPI2_SPIM2_SPIS2_IRQ                   35
#define NVIC_RTC2_IRQ                               36
#define NVIC_I2S_IRQ                                37
#define NVIC_FPU_IRQ                                38
#define NVIC_USBD_IRQ                               39
#define NVIC_UARTE1_IRQ                             40
#define NVIC_QSPI_IRQ                               41
#define NVIC_CRYPTOCELL_IRQ                         42
#define NVIC_PWM3_IRQ                               45
#define NVIC_SPIM3_IRQ                              47

/* --- POINTERS ---------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */

#define POWER       ((POWER_T *)  0x40000000)
#define CLOCK       ((CLOCK_T *)  0x40000000)
#define RADIO       ((RADIO_T *)  0x40001000)
#define SPIM0       ((SPIM_T  *)  0x40003000)
#define GPIOTE      ((GPIOTE_T *) 0x40006000)
#define TIMER0      ((TIMER_T *)  0x40008000)
#define TIMER1      ((TIMER_T *)  0x40009000)
#define TIMER2      ((TIMER_T *)  0x4000A000)
#define COMP        ((COMP_T  *)  0x40013000)
#define USBD        ((USBD_T  *)  0x40027000)
#define P0          ((GPIO_T  *)  0x50000000)
#define NVIC        ((NVIC_T  *)  0xE000E100)

#endif