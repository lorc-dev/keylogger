//
// Created by Lorenz on 20/02/2022.
//

#ifndef KEYLOGGER_M0PLUS_H
#define KEYLOGGER_M0PLUS_H

#include <stdint.h>
#include "../hardware/addressmap.h"

// ARM Cortex-M0+ registers (See Table 87 and ARM docs)
typedef struct {
    uint32_t volatile syst_csr;
    uint32_t volatile syst_rvr;
    uint32_t volatile syst_calib;
} m0plus_syst_hw_t;

typedef uint32_t volatile m0plus_nvic_ipr_hw_t[8];

typedef struct {
    uint32_t volatile cpuid;    // CPUID Base Register
    uint32_t volatile icsr;     // Interrupt Control and State Register
    uint32_t volatile vtor;     // Vector Table Offset Register
    uint32_t volatile aircr;    // Application Interrupt and Reset Control Register
    uint32_t volatile scr;      // System Control Register
    uint32_t volatile ccr;      // Configuration and Control Register
    uint32_t volatile shpr2;    // System Handler Priority Register 2
    uint32_t volatile shpr3;    // System Handler Priority Register 3
    uint32_t volatile shcsr;    // System Handler Control and State Register
} m0plus_scb_hw_t;

#define M0PLUS_SYST_OFFSET      0xe010  // SysTick
#define M0PLUS_NVIC_ISER_OFFSET 0xe100  // Interrupt Set-Enable Register
#define M0PLUS_NVIC_ICER_OFFSET 0xe180  // Interrupt Clear-Enable Register
#define M0PLUS_NVIC_ISPR_OFFSET 0xe200  // Interrupt Set-Pending Register
#define M0PLUS_NVIC_ICPR_OFFSET 0xe280  // Interrupt Clear-Pending Register
#define M0PLUS_NVIC_IPR_OFFSET  0xe400  // Interrupt Clear-Pending Register
#define M0PLUS_SCB_OFFSET       0xed00  // System Control Block


#define m0plus_syst_hw ((m0plus_syst_hw_t *) (PPB_BASE + M0PLUS_SYST_OFFSET))
#define m0plus_nvic_iser_hw ((uint32_t volatile *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET))
#define m0plus_nvic_icer_hw ((uint32_t volatile *) (PPB_BASE + M0PLUS_NVIC_ICER_OFFSET))
#define m0plus_nvic_ispr_hw ((uint32_t volatile *) (PPB_BASE + M0PLUS_NVIC_ISPR_OFFSET))
#define m0plus_nvic_icpr_hw ((uint32_t volatile *) (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET))
#define m0plus_nvic_ipr_hw ((m0plus_nvic_ipr_hw_t *) (PPB_BASE + M0PLUS_NVIC_IPR_OFFSET))
#define m0plus_scb_hw ((m0plus_scb_hw_t *) (PPB_BASE + M0PLUS_SCB_OFFSET))

#endif //KEYLOGGER_M0PLUS_H
