//
// Created by Lorenz on 22/10/2021.
//

#ifndef KEYLOGGER_CLOCKS_H
#define KEYLOGGER_CLOCKS_H

#include <stdint.h>
#include <stdbool.h>
#include "addressmap.h"

// Clocks registers (See Table 218)
typedef struct {
    struct{
       uint32_t volatile ctrl;
       uint32_t volatile div;
       uint32_t volatile selected;
    } clk[10];

    uint32_t volatile clk_sys_resus_ctrl;
    uint32_t volatile clk_sys_resus_status;

    uint32_t volatile fc0_ref_khz;
    uint32_t volatile fc0_min_khz;
    uint32_t volatile fc0_max_khz;
    uint32_t volatile fc0_delay;
    uint32_t volatile fc0_interval;
    uint32_t volatile fc0_src;
    uint32_t volatile fc0_status;
    uint32_t volatile fc0_result;

    uint32_t volatile wake_en0;
    uint32_t volatile wake_en1;
    uint32_t volatile sleep_en0;
    uint32_t volatile sleep_en1;
    uint32_t volatile enabled0;
    uint32_t volatile enabled1;
    uint32_t volatile intr;
    uint32_t volatile inte;
    uint32_t volatile intf;
    uint32_t volatile ints;
    
} clocks_hw_t;

#define clocks_hw ((clocks_hw_t *const)CLOCKS_BASE)

// CLK index
enum clock_index {
    clk_gpout0,
    clk_gpout1,
    clk_gpout2,
    clk_gpout3,
    clk_ref,
    clk_sys,
    clk_peri,
    clk_usb,
    clk_adc,
    clk_rtc,
};

// CLK registers (See Table 219 - 249)
#define CLK_CTRL_ENABLE_BIT (1ul<<11)

#define CLK_REF_CTRL_AUXSRC_VALUE_PLL_USB           0x0ul
#define CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS    0x0ul
#define CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB    0x0ul
#define CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB    0x0ul
#define CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB    0x0ul
#define CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS          0x0ul

#define CLK_SYS_REF_CTRL_SRC_VALUE_CLKSRC_CLK_AUX   0x1ul

#define CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC          0x2ul


#define CLK_SYS_CTRL_SRC_BITS       0x1ul
#define CLK_REF_CTRL_SRC_BITS       0x3ul
#define CLK_SYS_CTRL_AUXSRC_BITS    (0x5ul<<5)
#define CLK_REF_CTRL_AUXSRC_BITS    (0x3ul<<5)

#define CLK_CTRL_AUXSRC_LSB     5ul
#define CLK_CTRL_SRC_LSB        0ul


// Function prototypes
void clock_configure(enum clock_index clk_index, uint32_t src, uint32_t auxsrc, uint32_t src_freq, uint32_t freq);
void clocks_init(void);
uint32_t clock_get_hz(enum clock_index clk_index);
static inline bool has_glitchless_mux(enum clock_index clk_index);

#endif //KEYLOGGER_CLOCKS_H
