//
// Created by Lorenz on 17/10/2021.
//

#ifndef KEYLOGGER_GPIO_H
#define KEYLOGGER_GPIO_H

#include <stdint.h>
#include "addressmap.h"

// See table 289
#define GPIO_FUNC_SPI   1
#define GPIO_FUNC_UART  2
#define GPIO_FUNC_I2C   3
#define GPIO_FUNC_PWM   4
#define GPIO_FUNC_SIO   5
#define GPIO_FUNC_PIO0  6
#define GPIO_FUNC_PIO1  7
#define GPIO_FUNC_CLOCK 8
#define GPIO_FUNC_USB   9

// Pads bank0 gpio registers (Table 351)
#define PADS_BANK0_GPIO_SLEWFAST    (1ul)       // Slew rate control (1=fast, 0=slow)
#define PADS_BANK0_GPIO_SCHMITT     (1ul<<1)    // Enable schmitt trigger
#define PADS_BANK0_GPIO_PDE         (1ul<<2)    // Pull down enable
#define PADS_BANK0_GPIO_PUE         (1ul<<3)    // Pull up enable
#define PADS_BANK0_GPIO_DRIVE_12mA  (3ul<<4)    // Drive strength
#define PADS_BANK0_GPIO_DRIVE_8mA   (2ul<<4)
#define PADS_BANK0_GPIO_DRIVE_4mA   (1ul<<4)
#define PADS_BANK0_GPIO_DRIVE_2mA   (0ul<<4)
#define PADS_BANK0_GPIO_IE          (1ul<<6)    // Input enable
#define PADS_BANK0_GPIO_OD          (1ul<<7)    // Output disable

// See table 293
typedef struct {
    volatile uint32_t inte[4];  // Interrupt Enable
    volatile uint32_t intf[4];  // Interrupt Force
    volatile uint32_t ints[4];  // Interrupt Status
} io_int_hw_t;

typedef struct {
    struct {
        volatile uint32_t status;
        volatile uint32_t ctrl;
    } io[30];
    volatile uint32_t intr[4];  // Interrupt Raw
    io_int_hw_t proc0_irq_ctrl;
    io_int_hw_t proc1_irq_ctrl;
    io_int_hw_t dormant_wake_irq_ctrl;
} io_bank0_hw_t;
#define io_bank0_hw ((io_bank0_hw_t *const)IO_BANK0_BASE)


typedef struct {
    volatile uint32_t voltage_select;
    volatile uint32_t gpio[30];
} pads_bank0_hw_t;
#define pads_bank0_hw ((pads_bank0_hw_t *)PADS_BANK0_BASE)


// Function prototypes
void gpio_disable(void);
void gpio_enable(void);
void gpio_set_function(uint8_t gpio, uint8_t function);

#endif //KEYLOGGER_GPIO_H
