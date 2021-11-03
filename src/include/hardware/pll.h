//
// Created by Lorenz on 21/10/2021.
//

#ifndef KEYLOGGER_PLL_H
#define KEYLOGGER_PLL_H

#include <stdint.h>
#include "xosc.h"

// PLL registers (See Table 284)
typedef struct {
    uint32_t volatile cs;           // Control and status
    uint32_t volatile pwr;          // Controls the PLL power modes
    uint32_t volatile fbdiv_int;    // Feedback divisor
    uint32_t volatile prim;         // Controls the PLL post dividers for the primary output
} pll_hw_t;

#define pll_sys_hw ((pll_hw_t *const)PLL_SYS_BASE)
#define pll_usb_hw ((pll_hw_t *const)PLL_USB_BASE)

// PLL CS register
#define PLL_CS_LOCK_BIT     (1ul<<31)
#define PLL_CS_BYPASS_BIT   (1ul<<8)
#define PLL_CS_REFDIV_BITS  0x3ful

// PLL PWR register
#define PLL_PWR_VCOPD_BIT       (1ul<<5)
#define PLL_PWR_POSTDIVPD_BIT   (1ul<<3)
#define PLL_PWR_DSMPD_BIT       (1ul<<2)
#define PLL_PWR_PD_BIT          1ul

// PLL FBDIV_INT register
#define PLL_FBDIV_INT_BITS  0xffful

// PLL PRIM register
#define PLL_PRIM_POSTDIV1_BITS  0x70000ul  // Bits 18:16
#define PLL_PRIM_POSTDIV1_LSB   16
#define PLL_PRIM_POSTDIV2_BITS  0x7000ul   // Bits 14:12
#define PLL_PRIM_POSTDIV2_LSB   12


// VCO parameters
// See Python script: https://github.com/raspberrypi/pico-sdk/tree/master/src/rp2_common/hardware_clocks/scripts/vcocalc.py
#define USB_FDIV        (480ul/XOSC_MHZ)
#define USB_POST_DIV1   5ul
#define USB_POST_DIV2   2ul
#define USB_REFDIF      1ul

#define SYS_FDIV        (1500ul/XOSC_MHZ)
#define SYS_POST_DIV1   6ul
#define SYS_POST_DIV2   2ul
#define SYS_REFDIF      1ul


// Function prototypes
void pll_init();

#endif //KEYLOGGER_PLL_H
