//
// Created by Lorenz on 21/02/2022.
//

#include <stdint.h>
#include "../include/hardware/timer.h"

/**
 * Uptime in micro seconds
 *
 * @return time in µs
 */
uint64_t get_time(void) {
    uint32_t lo = timer_hw->timelr;
    uint32_t hi = timer_hw->timehr;
    return ((uint64_t) hi << 32u) | lo;
}

/**
 * Wait for the given number of µs.
 *
 * @param delay_us time in microseconds
 */
void wait_us_32(uint32_t delay_us) {
    uint32_t start = timer_hw->timerawl;
    while (timer_hw->timerawl - start < delay_us);
}

/**
 * Wait for the given number of µs
 *
 * @param delay_us time in microseconds
 */
void wait_us(uint64_t delay_us) {
    uint64_t start = get_time();
    uint64_t target = start + delay_us;
    uint32_t target_lo = (uint32_t) target;
    uint32_t target_hi = (uint32_t) (target >> 32u);
    while (timer_hw->timerawh < target_hi);
    while (timer_hw->timerawh == target_hi && timer_hw->timerawl < target_lo);
}

/**
 * Wait for the given number of ms
 *
 * @param delay_ms time in milliseconds
 */
void wait_ms(uint32_t delay_ms) {
    wait_us(delay_ms * 1000ull);
}