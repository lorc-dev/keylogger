//
// Created by Lorenz on 21/10/2021.
//

#ifndef KEYLOGGER_XOSC_H
#define KEYLOGGER_XOSC_H

#include <stdint.h>
#include "addressmap.h"

#define XOSC_MHZ 12

#define XOSC_CTRL_ENABLE_VALUE_DISABLE      0xd1eul         // Bit 23:12 of the CTRL register
#define XOSC_CTRL_ENABLE_VALUE_ENABLE       0xfabul
#define XOSC_CTRL_ENABLE_LSB                12ul            // Bit 23:12
#define XOSC_CTRL_ENABLE_BITS               0x00fff000ul
#define XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ  0xaa0ul         // Only possible value

#define XOSC_STATUS_STABLE_BIT              (1ul<<31)       // Will be set when the oscillator is running and stable

#define STARTUP_DELAY ((((XOSC_MHZ * 1000000) / 1000) / 256)    // =(fCrystal[Hz] * 1ms[s])/256 = +-47

// XOSC registers (See Table 268)
typedef struct {
    uint32_t ctrl;
    uint32_t status;
    uint32_t dormant;
    uint32_t startup;
    uint32_t _ud[3];    // Reserved, not in Table 268
    uint32_t count;
} xosc_hw_t;

#define xosc_hw ((xosc_hw_t *const)XOSC_BASE)


// Function prototypes
void xosc_init(void);


#endif //KEYLOGGER_XOSC_H
