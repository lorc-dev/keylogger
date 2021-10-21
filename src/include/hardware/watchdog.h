//
// Created by Lorenz on 21/10/2021.
//

#ifndef KEYLOGGER_WATCHDOG_H
#define KEYLOGGER_WATCHDOG_H

#include <stdint.h>
#include "addressmap.h"

#define WATCHDOG_TICK_ENABLE_BIT   (1ul<<9)

// Watchdog registers (See Table 555)
typedef struct {
    uint32_t volatile ctrl;
    uint32_t volatile load;
    uint32_t volatile reason;
    uint32_t volatile scratch[8];
    uint32_t volatile tick;
} watchdog_hw_t;

#define watchdog_hw ((watchdog_hw_t *const)WATCHDOG_BASE)


// Function prototypes
void watchdog_start_tick(uint8_t cycles);
#endif //KEYLOGGER_WATCHDOG_H
