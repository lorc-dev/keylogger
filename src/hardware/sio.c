//
// Created by Lorenz on 20/10/2021.
//
#include <stdbool.h>
#include "../include/hardware/sio.h"
#include "../include/hardware/gpio.h"


/**
 * Init a gpio pin as SIO
 *
 * @param gpio
 */
void sio_init(uint8_t gpio) {
    sio_hw->gpio_oe_clr = 1ul << gpio;      // Set the Output Enable bit for the selected gpio pin
    sio_hw->gpio_out_clr = 1ul << gpio;     // Set the output level low
    gpio_set_function(gpio, GPIO_FUNC_SIO); // Select the SIO function
}

/**
 * Set the output level of the selected gpio pin to value (1 = high, 0 = low)
 *
 * @param gpio
 * @param value
 */
void sio_put(uint8_t gpio, bool value) {
    uint32_t mask = 1ul << gpio;
    if (value)
        sio_out_set(mask);
    else
        sio_out_clr(mask);
}

/**
 * Inverts the output level of the selected gpio pin
 *
 * @param gpio
 */
void sio_toggle(uint8_t gpio) {
    uint32_t mask = 1ul << gpio;
    sio_out_xor(mask);
}

/**
 * Returns the input value of the chosen gpio pin
 *
 * @param gpio
 * @return
 */
bool sio_get(uint8_t gpio) {
    return ((1ul << gpio) & sio_hw->gpio_in) == (1ul << gpio);
}

/**
 * Set the direction (input/output) of the selected gpio pin
 *
 * @param gpio
 * @param out true = output, false = input
 */
void sio_set_dir(uint8_t gpio, bool out) {
    uint32_t mask = 1ul << gpio;
    if (out)
        sio_oe_set(mask);
    else
        sio_oe_clr(mask);
}


/**
 * Perform an atomic bit-set on gpio_out
 *
 * @param mask
 */
static inline void sio_out_set(uint32_t mask) {
    sio_hw->gpio_out_set = mask;
}

/**
 * Perform an atomic bit-clear on gpio_out
 *
 * @param mask
 */
static inline void sio_out_clr(uint32_t mask) {
    sio_hw->gpio_out_clr = mask;
}

/**
 * Perform an atomic bitwise XOR on gpio_out
 *
 * @param mask
 */
static inline void sio_out_xor(uint32_t mask) {
    sio_hw->gpio_out_xor = mask;
}

/**
 * Perform an atomic bit-set on gpio_oe
 *
 * @param mask
 */
static inline void sio_oe_set(uint32_t mask) {
    sio_hw->gpio_oe_set = mask;
}

/**
 * Perform an atomic bit-clear on gpio_oe
 *
 * @param mask
 */
static inline void sio_oe_clr(uint32_t mask) {
    sio_hw->gpio_oe_clr = mask;
}