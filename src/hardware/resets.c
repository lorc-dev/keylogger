//
// Created by Lorenz on 18/10/2021.
//

#include "../include/hardware/resets.h"

/**
 * Set specific reset bits
 *
 * @param bits
 */
void reset_subsystem(uint32_t bits)
{
    RESETS_RESET &= bits;
}

/**
 * Clear specific reset bits
 *
 * @param bits
 */
void unreset_subsystem(uint32_t bits)
{
    RESETS_RESET &= ~bits;
}

/**
 * Clear specific reset bits. Wait until the reset_done bits are set
 *
 * @param bits
 */
void unreset_subsystem_wait(uint32_t bits)
{
    RESETS_RESET &= ~bits;
    while((RESETS_RESET_DONE & bits) != bits);
}