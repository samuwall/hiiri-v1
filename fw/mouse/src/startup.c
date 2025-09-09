/********************************************************************
 ** file         : startup.c
 ** description  : copy .data, zero .bss, branch to main()
 **
 **
 ********************************************************************/

#include <stdint.h>

extern int main(void);
extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void reset_handler(void) {

    uint32_t *src = &_etext;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    /* nrf52820 errata [246] */
    *(volatile uint32_t *)0x4007AC84UL = 0x00000002UL;

    main();
    
    for(;;);
}

void default_handler(void) {
   for(;;);
}

/* core ISR declarations */
void nmi_handler(void)                           __attribute__ ((weak, alias ("default_handler")));
void hardfault_handler(void)                     __attribute__ ((weak, alias ("default_handler")));
void memmanage_fault_handler(void)               __attribute__ ((weak, alias ("default_handler")));
void busfault_handler(void)                      __attribute__ ((weak, alias ("default_handler")));
void usagefault_handler(void)                    __attribute__ ((weak, alias ("default_handler")));
void svc_handler(void)                           __attribute__ ((weak, alias ("default_handler")));
void debugmon_handler(void)                      __attribute__ ((weak, alias ("default_handler")));
void pendsv_handler(void)                        __attribute__ ((weak, alias ("default_handler")));
void systick_handler(void)                       __attribute__ ((weak, alias ("default_handler")));

/* nrf52840 ISR declarations */
void clock_power_isr(void)                       __attribute__ ((weak, alias ("default_handler")));
void radio_isr(void)                             __attribute__ ((weak, alias ("default_handler")));
void uart0_uarte0_isr(void)                      __attribute__ ((weak, alias ("default_handler")));
void spi0_spim0_spis0_twi0_twim0_twis0_isr(void) __attribute__ ((weak, alias ("default_handler")));
void spi1_spim1_spis1_twi1_twim1_twis1_isr(void) __attribute__ ((weak, alias ("default_handler")));
void nfct_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void gpiote_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void saadc_isr(void)                             __attribute__ ((weak, alias ("default_handler")));
void timer0_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void timer1_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void timer2_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void rtc0_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void temp_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void rng_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void ecb_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void aar_ccm_isr(void)                           __attribute__ ((weak, alias ("default_handler")));
void wdt_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void rtc1_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void qdec_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void comp_lpcomp_isr(void)                       __attribute__ ((weak, alias ("default_handler")));
void egu0_swi0_isr(void)                         __attribute__ ((weak, alias ("default_handler")));
void egu1_swi1_isr(void)                         __attribute__ ((weak, alias ("default_handler")));
void egu2_swi2_isr(void)                         __attribute__ ((weak, alias ("default_handler")));
void egu3_swi3_isr(void)                         __attribute__ ((weak, alias ("default_handler")));
void egu4_swi4_isr(void)                         __attribute__ ((weak, alias ("default_handler")));
void egu5_swi5_isr(void)                         __attribute__ ((weak, alias ("default_handler")));
void timer3_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void timer4_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void pwm0_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void pdm_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void mwu_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void pwm1_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void pwm2_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void spi2_spim2_spis2_isr(void)                  __attribute__ ((weak, alias ("default_handler")));
void rtc2_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void i2s_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void fpu_isr(void)                               __attribute__ ((weak, alias ("default_handler")));
void usbd_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void uarte1_isr(void)                            __attribute__ ((weak, alias ("default_handler")));
void qspi_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void cryptocell_isr(void)                        __attribute__ ((weak, alias ("default_handler")));
void pwm3_isr(void)                              __attribute__ ((weak, alias ("default_handler")));
void spim3_isr(void)                             __attribute__ ((weak, alias ("default_handler")));

const uint32_t vector_table[]
__attribute__ ((used, section (".vectors"))) = {
    (uint32_t) &_estack,
    (uint32_t) reset_handler,
    (uint32_t) nmi_handler,
    (uint32_t) hardfault_handler,
    (uint32_t) memmanage_fault_handler,
    (uint32_t) busfault_handler,
    (uint32_t) usagefault_handler,
    (uint32_t) 0,
    (uint32_t) 0,
    (uint32_t) 0,
    (uint32_t) 0,
    (uint32_t) svc_handler,
    (uint32_t) debugmon_handler,
    (uint32_t) 0,
    (uint32_t) pendsv_handler,
    (uint32_t) systick_handler,

    /* external interrupts, IRQ0..IRQ47 */
    (uint32_t) clock_power_isr,
    (uint32_t) radio_isr,
    (uint32_t) uart0_uarte0_isr,
    (uint32_t) spi0_spim0_spis0_twi0_twim0_twis0_isr,
    (uint32_t) spi1_spim1_spis1_twi1_twim1_twis1_isr,
    (uint32_t) nfct_isr,
    (uint32_t) gpiote_isr,
    (uint32_t) saadc_isr,
    (uint32_t) timer0_isr,
    (uint32_t) timer1_isr,
    (uint32_t) timer2_isr,
    (uint32_t) rtc0_isr,
    (uint32_t) temp_isr,
    (uint32_t) rng_isr,
    (uint32_t) ecb_isr,
    (uint32_t) aar_ccm_isr,
    (uint32_t) wdt_isr,
    (uint32_t) rtc1_isr,
    (uint32_t) qdec_isr,
    (uint32_t) comp_lpcomp_isr,
    (uint32_t) egu0_swi0_isr,
    (uint32_t) egu1_swi1_isr,
    (uint32_t) egu2_swi2_isr,
    (uint32_t) egu3_swi3_isr,
    (uint32_t) egu4_swi4_isr,
    (uint32_t) egu5_swi5_isr,
    (uint32_t) timer3_isr,
    (uint32_t) timer4_isr,
    (uint32_t) pwm0_isr,
    (uint32_t) pdm_isr,
    (uint32_t) 0,
    (uint32_t) 0,
    (uint32_t) mwu_isr,
    (uint32_t) pwm1_isr,
    (uint32_t) pwm2_isr,
    (uint32_t) spi2_spim2_spis2_isr,
    (uint32_t) rtc2_isr,
    (uint32_t) i2s_isr,
    (uint32_t) fpu_isr,
    (uint32_t) usbd_isr,
    (uint32_t) uarte1_isr,
    (uint32_t) qspi_isr,
    (uint32_t) cryptocell_isr,
    (uint32_t) 0,
    (uint32_t) 0,
    (uint32_t) pwm3_isr,
    (uint32_t) 0,
    (uint32_t) spim3_isr,
};

