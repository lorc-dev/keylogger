//
// Created by Lorenz on 21/10/2021.
//

#include <stdint.h>
#include "../include/hardware/pll.h"
#include "../include/hardware/resets.h"

/**
 * Initialize the usb and sys pll
 * USB: 48MHz
 * SYS: 125MHz
 */
void pll_init() {
    // 1. Program the reference clock divider
    // 2. Program the feedback divider
    // 3. Turn on the main and VCO power
    // 4. Wait for the VCO to lock (i.e. keep its output freq stable)
    // 5. Set up post dividers and turn them on

    // Combine div1 and div2 with the correct bit positions in the PRIM register
    uint32_t usb_pdiv = (USB_POST_DIV1 << PLL_PRIM_POSTDIV1_LSB) | (USB_POST_DIV2 << PLL_PRIM_POSTDIV2_LSB);
    uint32_t sys_pdiv = (SYS_POST_DIV1 << PLL_PRIM_POSTDIV1_LSB) | (SYS_POST_DIV2 << PLL_PRIM_POSTDIV2_LSB);

    // Reset the subsystems
    reset_subsystem(RESETS_RESET_PLL_USB);
    reset_subsystem(RESETS_RESET_PLL_SYS);
    unreset_subsystem_wait(RESETS_RESET_PLL_USB);
    unreset_subsystem_wait(RESETS_RESET_PLL_SYS);

    // Set REFDIF and FBDIV_INT.
    //  - This will clear the BYPASS bit, but this is already the case after a reset.
    pll_usb_hw->cs = USB_REFDIF;
    pll_usb_hw->fbdiv_int = USB_FDIV;
    pll_sys_hw->cs = SYS_REFDIF;
    pll_sys_hw->fbdiv_int = SYS_FDIV;

    // Turn Main and VCO power on
    uint32_t power = PLL_PWR_PD_BIT | PLL_PWR_VCOPD_BIT;
    pll_usb_hw->pwr &= ~power;
    pll_sys_hw->pwr &= ~power;

    // Wait for the VCO to lock (stable output freq)
    while ((pll_usb_hw->cs & PLL_CS_LOCK_BIT) != PLL_CS_LOCK_BIT &&
            (pll_sys_hw->cs & PLL_CS_LOCK_BIT) != PLL_CS_LOCK_BIT);

    // Set up post dividers
    pll_usb_hw->prim = usb_pdiv;
    pll_sys_hw->prim = sys_pdiv;

    // Turn on post divider
    pll_usb_hw->pwr &= ~PLL_PWR_POSTDIVPD_BIT;
    pll_sys_hw->pwr &= ~PLL_PWR_POSTDIVPD_BIT;

}
