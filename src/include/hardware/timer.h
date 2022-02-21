//
// Created by Lorenz on 21/02/2022.
//

#ifndef KEYLOGGER_TIMER_H
#define KEYLOGGER_TIMER_H

#include <stdint.h>
#include "addressmap.h"

typedef struct {
    uint32_t volatile timehw;
    uint32_t volatile timelw;
    uint32_t volatile timehr;
    uint32_t volatile timelr;
    uint32_t volatile alarm[4];
    uint32_t volatile armed;
    uint32_t volatile timerawh;
    uint32_t volatile timerawl;
    uint32_t volatile dbgpause;
    uint32_t volatile pause;
    uint32_t volatile intr;
    uint32_t volatile inte;
    uint32_t volatile intf;
    uint32_t volatile ints;
} timer_hw_t;

#define timer_hw ((timer_hw_t *const)TIMER_BASE)

// Function prototypes
uint64_t get_time(void);
void wait_us_32(uint32_t delay_us);
void wait_us(uint64_t delay_us);
void wait_ms(uint32_t delay_ms);

#endif //KEYLOGGER_TIMER_H
