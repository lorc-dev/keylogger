//
// Created by Lorenz on 20/10/2021.
//

#ifndef KEYLOGGER_SIO_H
#define KEYLOGGER_SIO_H

#include <stdint.h>
#include <stdbool.h>
#include "addressmap.h"

#define INPUT 0
#define OUTPUT 1

/**
 * SIO registers (See Table 16)
 *  - gpio_    : GPIO pins
 *  - gpio_hi  : QSPI pins
 */
typedef struct {
    uint32_t volatile cpuid;

    uint32_t volatile gpio_in;
    uint32_t volatile gpio_hi_in;

    uint32_t volatile _ud;  // Not listed in Table 16 (undefined)

    uint32_t volatile gpio_out;
    uint32_t volatile gpio_out_set;
    uint32_t volatile gpio_out_clr;
    uint32_t volatile gpio_out_xor;

    uint32_t volatile gpio_oe;
    uint32_t volatile gpio_oe_set;
    uint32_t volatile gpio_oe_clr;
    uint32_t volatile gpio_oe_xor;

    uint32_t volatile gpio_hi_out;
    uint32_t volatile gpio_hi_set;
    uint32_t volatile gpio_hi_clr;
    uint32_t volatile gpio_hi_xor;

    uint32_t volatile gpio_hi_oe;
    uint32_t volatile gpio_hi_oe_set;
    uint32_t volatile gpio_hi_oe_clr;
    uint32_t volatile gpio_hi_oe_xor;

    // Incomplete...

} sio_hw_t;

#define sio_hw ((sio_hw_t *)SIO_BASE)

// Function prototypes
void sio_init(uint8_t gpio);
void sio_put(uint8_t gpio, bool value);
void sio_toggle(uint8_t gpio);
bool sio_get(uint8_t gpio);
void sio_set_dir(uint8_t gpio, bool out);

static inline void sio_out_set(uint32_t mask);
static inline void sio_out_clr(uint32_t mask);
static inline void sio_out_xor(uint32_t mask);
static inline void sio_oe_set(uint32_t mask);
static inline void sio_oe_clr(uint32_t mask);

#endif //KEYLOGGER_SIO_H
