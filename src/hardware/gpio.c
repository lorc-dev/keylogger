//
// Created by Lorenz on 18/10/2021.
//

#include "../include/hardware/gpio.h"
#include "../include/hardware/resets.h"

/**
 * Disable the gpio subsystem
 */
void gpio_disable(void)
{
    reset_subsystem(RESETS_RESET_IO_BANK0 | RESETS_RESET_PAD_BANK0);
}

/**
 * Enable the gpio subsystem
 */
void gpio_enable(void)
{
    unreset_subsystem_wait(RESETS_RESET_IO_BANK0 | RESETS_RESET_PAD_BANK0);
}

/**
 * Assign a function to a gpio pin. See the GPIO function table (Table 289)
 *
 * @param gpio
 * @param function
 */
void gpio_set_function(uint8_t gpio, uint8_t function)
{
    pads_bank0_hw->gpio[gpio] &= ~PADS_BANK0_GPIO_OD;   // Clear output disable
    pads_bank0_hw->gpio[gpio] |= PADS_BANK0_GPIO_IE;    // Set input enable
    io_bank0_hw->io[gpio].ctrl = function;              // Clear all bits and set the FUNCSEL bits (4:0)
}