//
// Created by Lorenz on 21/10/2021.
//

#include "../include/hardware/xosc.h"

/**
 * Init the Crystal Oscillator (XOSC)
 */
void xosc_init(void) {
    // Set the frequency range (1-15MHz).
    xosc_hw->ctrl = XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ;

    // Set xosc startup delay bit 13:0
    xosc_hw->startup = STARTUP_DELAY;

    // Enable XOSC (Set the enable bit)
    xosc_hw->ctrl |= XOSC_CTRL_ENABLE_VALUE_ENABLE << XOSC_CTRL_ENABLE_LSB;

    // Wait for XOSC to be stable
    while((xosc_hw->status & XOSC_STATUS_STABLE_BIT) != XOSC_STATUS_STABLE_BIT);
}