//
// Created by Lorenz on 27/10/2021.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/hardware/uart.h"
#include "../include/hardware/resets.h"
#include "../include/hardware/clocks.h"
#include "../include/lib/math.h"

// The RP2040 datasheet is incomplete
// See PrimeCell UART (PL011) Technical reference Manual
// https://developer.arm.com/documentation/ddi0183/latest/

/**
 * Initializes uart0 or uart1 and sets the baud rate
 * @note clk_peri >= 16 * max baud rate
 * @note clk_peri <= 16 * 65535 * min baud rate
 *
 * @param uart
 * @param baudrate Max 921600 bps
 */
void uart_init(uart_hw_t *uart, uint32_t baudrate) {
    uint32_t uart_reset_select_mask;
    if(uart == uart0_hw)
        uart_reset_select_mask = RESETS_RESET_UART0;
    else
        uart_reset_select_mask = RESETS_RESET_UART1;

    reset_subsystem(uart_reset_select_mask);
    unreset_subsystem_wait(uart_reset_select_mask);

    uart_set_baudrate(uart, baudrate);
    uart_set_format(uart, 8, 1);

    // Enable the UART, both TX and RX
    uart->uartcr |= UART_UARTCR_UARTEN_BIT | UART_UARTCR_TXE_BIT | UART_UARTCR_RXE_BIT;
    // Enable FIFOs
    uart->uartlcr_h |= UART_UARTLCR_H_FEN_BIT;
}

/**
 * Configures the baud rate.
 * @note clk_peri >= 16 * max baud rate
 * @note clk_peri <= 16 * 65535 * min baud rate
 *
 * @param uart
 * @param baudrate Max 921600 bps
 */
void uart_set_baudrate(uart_hw_t *uart, uint32_t baudrate) {
    // Fractional baud rate divider (BRDi + BRDf) (See 4.2.3.2.1)
    // BRDi = 16 bit integer part
    // BRDf = 6 bit fractional part
    // BRD = UARTCLK / (16 * baud rate)

    // Shifted 7 bytes right (*128)
    uint32_t baud_rate_div = integer_division(8 * clock_get_hz(clk_peri), baudrate);
    // Get the integer part
    uint32_t baud_ibrd = baud_rate_div >> 7;
    // Get the fractional part (BRDf * 64 + 0.5)
    uint32_t baud_fbrd = ((baud_rate_div & 0x7f) + 1) >> 1;

    // The maximum divide ratio is 65535 (0xffff)
    if (baud_ibrd >= 0xffff) {
        baud_ibrd = 0xffff;
        baud_fbrd = 0;
    }
    // The minimum divide ratio is 1
    else if (baud_ibrd == 0) {
        baud_ibrd = 1;
        baud_fbrd = 0;
    }

    // Load baud divisor registers
    uart->uartibrd = baud_ibrd;
    uart->uartfbrd = baud_fbrd;
}

/**
 * Configures the line control register
 *
 * @param uart
 * @param data_bits 5-8bits
 * @param stop_bits 1-2bits
 */
static inline void uart_set_format(uart_hw_t *uart, uint8_t data_bits, uint8_t stop_bits) {
    uint32_t value = (data_bits - 5ul) << UART_UARTLCR_H_WLEN_LSB |
                     (stop_bits - 1ul) << UART_UARTLCR_H_STP2_LSB;

    uint32_t mask = UART_UARTLCR_H_WLEN_BITS | UART_UARTLCR_H_STP2_BIT;

    // Write the value with mask
    uart->uartlcr_h = (uart->uartlcr_h & ~mask) | (value & mask);
}

/**
 * Checks if the FIFO buffer is not full
 *
 * @param uart
 * @return True when FIFO buffer is not full, False when FIFO is full
 */
static inline bool uart_is_writable(uart_hw_t *uart) {
    return !(uart->uartfr & UART_UARTFR_TXFF_BIT);
}

/**
 * Checks if there is data in the FIFO buffer
 *
 * @param uart
 * @return True when the buffer is not empty, False when empty
 */
static inline bool uart_is_readable(uart_hw_t *uart) {
    return !(uart->uartfr & UART_UARTFR_RXFE_BIT);
}

/**
 * Writes a buffer(*src) to the uart FIFO
 * @note This function will wait until all data is sent
 *
 * @param uart
 * @param src
 * @param len
 */
void uart_write(uart_hw_t *uart, uint8_t *src, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!uart_is_writable(uart));    // Wait until there is space in the FIFO buffer
        uart->uartdr = *src++;
    }
}

/**
 * Reads data from the FIFO buffer into *dst
 * @note This function will wait until the given length is read
 *
 * @param uart
 * @param dst
 * @param len
 */
void uart_read(uart_hw_t *uart, uint8_t *dst, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!uart_is_readable(uart));    // Wait until there is data in the buffer
        *dst++ = (uint8_t) uart->uartdr;
    }
}

/**
 * Write a single char to UART
 * @note This function will wait until the char has been sent
 *
 * @param uart
 * @param c
 */
void uart_putc(uart_hw_t *uart, uint8_t c) {
    uart_write(uart, &c, 1);
}

/**
 * Reads a single char from UART
 * @note This function will wait until the char has been read
 *
 * @param uart
 * @return
 */
uint8_t uart_getc(uart_hw_t * uart) {
    uint8_t c;
    uart_read(uart, &c, 1);
    return c;
}

/**
 * Writes a string (null byte ending) to UART
 * @note This function will wait until the char has been sent
 *
 * @param uart
 * @param str
 */
void uart_puts(uart_hw_t *uart, char *str) {
    while(*str)
        uart_putc(uart,*str++);
}