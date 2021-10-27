//
// Created by Lorenz on 26/10/2021.
//

#include "../include/runtime/runtime.h"
#include "../include/hardware/resets.h"
#include "../include/hardware/clocks.h"

/**
 * Initializes the runtime. This should be the first function that is called
 */
void runtime_init(void) {
    // Reset all peripherals to put system into a known state,
    // - except for QSPI pads and the XIP IO bank, as this is fatal if running from flash
    // - and the PLLs, as this is fatal if clock muxing has not been reset on this boot
    // - and USB, syscfg, as this disturbs USB-to-SWD on core 1
    reset_subsystem(~(
            RESETS_RESET_IO_QSPI|
            RESETS_RESET_PADS_QSPI |
            RESETS_RESET_PLL_USB |
            RESETS_RESET_USBCTRL |
            RESETS_RESET_SYSCFG |
            RESETS_RESET_PLL_SYS
    ));

    // Remove reset from peripherals which are clocked only by clk_sys and
    // clk_ref. Other peripherals stay in reset until we've configured clocks.
    unreset_subsystem_wait(RESETS_RESET_BITS & ~(
            RESETS_RESET_ADC |
            RESETS_RESET_RTC |
            RESETS_RESET_SPI0 |
            RESETS_RESET_SPI1 |
            RESETS_RESET_UART0 |
            RESETS_RESET_UART1 |
            RESETS_RESET_USBCTRL
    ));

    // Configure the clocks
    clocks_init();

    // Un-reset all peripherals
    unreset_subsystem_wait(RESETS_RESET_BITS);
}