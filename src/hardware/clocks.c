//
// Created by Lorenz on 22/10/2021.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/hardware/clocks.h"
#include "../include/hardware/watchdog.h"
#include "../include/hardware/xosc.h"
#include "../include/hardware/pll.h"
#include "../include/lib/math.h"

// Space to save the configured clock frequencies
// Global variable -> automatically initialized with 0
static int configured_freq[10];

/**
 * Configure a clock.
 * @NOTE: The system clock need to be configured before other clocks without glithcless mux are
 *
 * @param clk_index
 * @param src
 * @param auxsrc
 * @param src_freq
 * @param freq Needs to be smaller than the source frequency
 */
void clock_configure(enum clock_index clk_index, uint32_t src, uint32_t auxsrc, uint32_t src_freq, uint32_t freq) {

    // 24.8 int.fraq divider -> shift 8 left.
    // to avoid 64bit integers, we don't use the fractional part
    uint32_t div = integer_division(src_freq, freq) << 8;

    // When the divisior is increased, avoid momentary overspeed setting the divisor first
    if (div > clocks_hw->clk[clk_index].div)
        clocks_hw->clk[clk_index].div = div;

    // If switching a glitchless mux to an aux source
    //  - Switch to an alternate source
    if (has_glitchless_mux(clk_index) && src == CLK_SYS_REF_CTRL_SRC_VALUE_CLKSRC_CLK_AUX) {
        uint32_t mask;
        if (clk_index == clk_ref)
            mask = ~CLK_REF_CTRL_SRC_BITS;
        else
            mask = ~CLK_SYS_CTRL_SRC_BITS;

        clocks_hw->clk[clk_index].ctrl &= mask;     // Switch the glitchless mux to an altrnate source (0x0 is never an aux source)
        while(!clocks_hw->clk[clk_index].selected); // Poll the SELECTED register until the switch is completed
    }
    // No glitchless mux
    //  - Stop the clock
    else {
        clocks_hw->clk[clk_index].ctrl &= ~CLK_CTRL_ENABLE_BIT;     // Disable the clock generator

        // Enable propagation can take up to 3 cycles.
        // Delay for 3 cycles of the target clock (see phase register)
        // This only works if the system clock is already configured
        for (uint32_t i = 0; i < integer_division(configured_freq[clk_sys], configured_freq[clk_index]) * 3; i++)
            asm volatile ("nop");   // Nop instruction execution time is exactly 1 sys clock cycle
    }

    // Set the aux mux
    clocks_hw->clk[clk_index].ctrl = (clocks_hw->clk[clk_index].ctrl & ~CLK_SYS_CTRL_AUXSRC_BITS) | ((auxsrc << CLK_CTRL_AUXSRC_LSB) & CLK_SYS_CTRL_AUXSRC_BITS);

    // Set the glitchless mux if the clock has one
    if(has_glitchless_mux(clk_index)) {
        clocks_hw->clk[clk_index].ctrl = (clocks_hw->clk[clk_index].ctrl & ~CLK_CTRL_SRC_BITS) | ((src << CLK_CTRL_SRC_LSB) & CLK_CTRL_SRC_BITS);
        while(!clocks_hw->clk[clk_index].selected);  // Poll the SELECTED register until the switch is completed
    }

    // Enable the clock
    clocks_hw->clk[clk_index].ctrl |= CLK_CTRL_ENABLE_BIT;

    // Set the divisor
    clocks_hw->clk[clk_index].div = div;

    // Store the configured frequency for later use
    configured_freq[clk_index] = freq;

}

/**
 * Initializes all the clocks
 */
void clocks_init(void) {
    // Start tick in watchdog
    watchdog_start_tick(XOSC_MHZ);

    // Enable the xosc
    xosc_init();

    // Switch sys and ref away from their aux sources.
    clocks_hw->clk[clk_sys].ctrl &= ~CLK_SYS_CTRL_SRC_BITS;
    while (!clocks_hw->clk[clk_sys].selected);

    clocks_hw->clk[clk_ref].ctrl &= ~CLK_REF_CTRL_SRC_BITS;
    while (!clocks_hw->clk[clk_ref].selected);

    // Configure the plls
    pll_init();

    // Configure clocks
    // CLK_REF = XOSC (12MHz)
    clock_configure(clk_ref, CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0, XOSC_MHZ * 1000000, XOSC_MHZ * 1000000);

    // CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
    clock_configure(clk_sys, CLK_SYS_REF_CTRL_SRC_VALUE_CLKSRC_CLK_AUX, CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 125 * 1000000, 125 * 1000000);

    // CLK USB = PLL USB (48MHz) / 1 = 48MHz
    clock_configure(clk_usb, 0, CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * 1000000, 48 * 1000000);

    // CLK ADC = PLL USB (48MHZ) / 1 = 48MHz
    clock_configure(clk_adc, 0, CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * 1000000, 48 * 1000000);

    // CLK RTC = PLL USB (48MHz) / 1024 = 46875Hz
    clock_configure(clk_rtc, 0, CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * 1000000, 46875);

    // CLK PERI = clk_sys
    clock_configure(clk_peri, 0, CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, 125 * 1000000, 125 * 1000000);

}

/**
 * Returns the configured frequency set in clock_configure()
 *
 * @param clk_index
 * @return
 */
uint32_t clock_get_hz(enum clock_index clk_index) {
    return  configured_freq[clk_index];
}

/**
 * Checks if the given clock is sys or ref
 *
 * @param clk_index
 * @return
 */
static inline bool has_glitchless_mux(enum clock_index clk_index) {
    return clk_index == clk_sys || clk_index == clk_ref;
}