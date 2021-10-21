//
// Created by Lorenz on 21/10/2021.
//

#include <stdint.h>
#include "../include/hardware/watchdog.h"

/**
 * Starts the watchdog tick. The tick is also used for the timer
 *
 * @param cycles Total number of clk_tick cycles before the next tick
 */
void watchdog_start_tick(uint8_t cycles) {
    // Set the number of clk_tick cycles before the next tick. And enable tick generation
    // Bits 31:10 are read only.
    watchdog_hw->tick = cycles | WATCHDOG_TICK_ENABLE_BIT;
}
