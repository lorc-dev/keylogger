//
// Created by Lorenz on 27/10/2021.
//

#ifndef KEYLOGGER_UART_H
#define KEYLOGGER_UART_H

#include <stdint.h>
#include <stdbool.h>
#include "addressmap.h"

// UART registers (See Table 435)
typedef struct {
    uint32_t volatile uartdr;       // Data
    uint32_t volatile uartrsr;      // Receive status
    uint32_t _undef[4];
    uint32_t volatile uartfr;       // Flag
    uint32_t _undef_1;
    uint32_t volatile uartilpr;     // IrDA low-power counter
    uint32_t volatile uartibrd;     // Integer baud rate
    uint32_t volatile uartfbrd;     // Fractional baud rate
    uint32_t volatile uartlcr_h;    // Line control
    uint32_t volatile uartcr;       // Control
    uint32_t volatile uartifls;     // Interrupt FIFO level select
    uint32_t volatile uartimsc;     // Interrupt mask set/clear
    uint32_t volatile uartris;      // Raw interrupt status
    uint32_t volatile uartmis;      // Masked interrupt status
    uint32_t volatile uarticr;      // Interrupt clear
    uint32_t volatile uartdmacr;    // DMA control
} uart_hw_t;

#define uart0_hw ((uart_hw_t *const)UART0_BASE)
#define uart1_hw ((uart_hw_t *const)UART1_BASE)


// UARTCR register (See Table 443)
#define UART_UARTCR_UARTEN_BIT 1ul         // UART enable
#define UART_UARTCR_TXE_BIT    (1ul<<8)    // Transmit enable
#define UART_UARTCR_RXE_BIT    (1ul<<9)    // Receive enable

// UARTLCR_H register (See Table 442)
#define UART_UARTLCR_H_WLEN_BITS    (3ul<<5)    // Word length (00 = 5bit, 01 = 6bit, 10 = 7bit, 11 = 8bit)
#define UART_UARTLCR_H_FEN_BIT      (1ul<<4)    // Enable FIFO's
#define UART_UARTLCR_H_STP2_BIT     (1ul<<3)    // Two stop bits select
#define UART_UARTLCR_H_EPS_BIT      (1ul<<2)    // Even parity select
#define UART_UARTLCR_H_PEN_BIT      (1ul<<1)    // Parity enable
#define UART_UARTLCR_H_WLEN_LSB     5
#define UART_UARTLCR_H_FEN_LSB      4
#define UART_UARTLCR_H_STP2_LSB     3
#define UART_UARTLCR_H_EPS_LSB      2
#define UART_UARTLCR_H_PEN_LSB      1

// UARTIBRD register (See Table 440)
#define UART_UARTIBRD_BITS  0xfffful    // Integer baud register bits

// UARTFBRD register (See Table 441)
#define UART_UARTFBRD_BITS  0x3f        // Fractional baud register bits

// UARTFR register (See Table 438)
#define UART_UARTFR_TXFE_BIT   (1ul<<7)    // Transmit FIFO empty
#define UART_UARTFR_RXFF_BIT   (1ul<<6)    // Receive FIFO full
#define UART_UARTFR_TXFF_BIT   (1ul<<5)    // Transmit FIFO full
#define UART_UARTFR_RXFE_BIT   (1ul<<4)    // Receive FIFO empty


// Function prototypes
void uart_init(uart_hw_t *uart, uint32_t baudrate);
void uart_set_baudrate(uart_hw_t *uart, uint32_t baudrate);
static inline void uart_set_format(uart_hw_t *uart, uint8_t data_bits, uint8_t stop_bits);
static inline bool uart_is_writable(uart_hw_t *uart);
static inline bool uart_is_readable(uart_hw_t *uart);
void uart_write(uart_hw_t *uart, uint8_t *src, uint32_t len);
void uart_read(uart_hw_t *uart, uint8_t *dst, uint32_t len);
void uart_putc(uart_hw_t *uart, uint8_t c);
uint8_t uart_getc(uart_hw_t * uart);
void uart_puts(uart_hw_t *uart, char *str);

#endif //KEYLOGGER_UART_H
