//
// Created by Lorenz on 20/02/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/hardware/irq.h"
#include "../include/hardware/m0plus.h"

extern void __unhandled_user_irq(void);

/**
 * Enable or disable a interrupt
 *
 * @param num
 * @param enabled
 */
void irq_set_enabled(uint32_t num, bool enabled) {
    uint32_t mask = 1ul << num;
    if (enabled) {
        *m0plus_nvic_icpr_hw = mask;    // Clear pending
        *m0plus_nvic_iser_hw = mask;    // Enable
    } else {
        *m0plus_nvic_icer_hw = mask;    // Clear enable
    }
}

static inline irq_handler_t *get_vtable(void) {
    return (irq_handler_t *) m0plus_scb_hw->vtor;   // TODO: check lsb
}


/**
 * Set an interrupt handler in the vtable
 *
 * @param num
 * @param handler
 */
void irq_set_handler(uint32_t num, irq_handler_t handler) {
    get_vtable()[num + 16] = handler;
}