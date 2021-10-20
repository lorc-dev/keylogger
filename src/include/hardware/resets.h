//
// Created by Lorenz on 17/10/2021.
//

#ifndef KEYLOGGER_RESETS_H
#define KEYLOGGER_RESETS_H

#include <stdint.h>
#include "addressmap.h"
#include "../common/common.h"

// See 2.14.3 Reset Controller
// TODO: Change to struct
#define RESETS_RESET        ADDR(RESETS_BASE + 0x0)
#define RESETS_WDSEL        ADDR(RESETS_BASE + 0x4)
#define RESETS_RESET_DONE   ADDR(RESETS_BASE + 0x8)

// RESET Register
#define RESETS_RESET_ADC            1ul
#define RESETS_RESET_BUSCTRL        (1ul<<1)
#define RESETS_RESET_DMA            (1ul<<2)
#define RESETS_RESET_I2C0           (1ul<<3)
#define RESETS_RESET_I2C1           (1ul<<4)
#define RESETS_RESET_IO_BANK0	    (1ul<<5)
#define RESETS_RESET_IO_QSPI        (1ul<<6)
#define RESETS_RESET_JTAG           (1ul<<7)
#define RESETS_RESET_PAD_BANK0	    (1ul<<8)
#define RESETS_RESET_PADS_QSPI      (1ul<<9)
#define RESETS_RESET_PIO0           (1ul<<10)
#define RESETS_RESET_PIO1           (1ul<<11)
#define RESETS_RESET_PLL_SYS        (1ul<<12)
#define RESETS_RESET_PLL_USB        (1ul<<13)
#define RESETS_RESET_PWM            (1ul<<14)
#define RESETS_RESET_RTC            (1ul<<15)
#define RESETS_RESET_SPI0           (1ul<<16)
#define RESETS_RESET_SPI1           (1ul<<17)
#define RESETS_RESET_SYSCFG         (1ul<<18)
#define RESETS_RESET_SYSINFO        (1ul<<19)
#define RESETS_RESET_TBMAN          (1ul<<20)
#define RESETS_RESET_TIMER          (1ul<<21)
#define RESETS_RESET_UART0          (1ul<<22)
#define RESETS_RESET_UART1          (1ul<<23)
#define RESETS_RESET_USBCTRL        (1ul<<24)

void reset_subsystem(uint32_t bits);
void unreset_subsystem(uint32_t bits);
void unreset_subsystem_wait(uint32_t bits);

#endif //KEYLOGGER_RESETS_RESET_H
