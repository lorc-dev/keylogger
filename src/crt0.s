.syntax unified
.cpu cortex-m0plus
.thumb

// Vector table
.section .vectors, "ax"
.align 2 

.global __vectors, __VECTOR_TABLE
__VECTOR_TABLE:
__vectors:
.word __StackTop  
.word _reset_handler
.word isr_nmi
.word isr_hardfault
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_svcall
.word isr_invalid // Reserved, should never fire
.word isr_invalid // Reserved, should never fire
.word isr_pendsv
.word isr_systick
.word isr_irq0
.word isr_irq1
.word isr_irq2
.word isr_irq3
.word isr_irq4
.word isr_irq5
.word isr_irq6
.word isr_irq7
.word isr_irq8
.word isr_irq9
.word isr_irq10
.word isr_irq11
.word isr_irq12
.word isr_irq13
.word isr_irq14
.word isr_irq15
.word isr_irq16
.word isr_irq17
.word isr_irq18
.word isr_irq19
.word isr_irq20
.word isr_irq21
.word isr_irq22
.word isr_irq23
.word isr_irq24
.word isr_irq25
.word isr_irq26
.word isr_irq27
.word isr_irq28
.word isr_irq29
.word isr_irq30
.word isr_irq31

// Make weak symbol for each ISR
// Can be overridden, by default they will be handled to the undifined IRQ handler
.macro decl_isr name
.weak \name
.type \name,%function
.thumb_func
\name:
.endm

decl_isr isr_invalid
decl_isr isr_nmi
decl_isr isr_hardfault
decl_isr isr_svcall
decl_isr isr_pendsv
decl_isr isr_systick
decl_isr isr_irq0
decl_isr isr_irq1
decl_isr isr_irq2
decl_isr isr_irq3
decl_isr isr_irq4
decl_isr isr_irq5
decl_isr isr_irq6
decl_isr isr_irq7
decl_isr isr_irq8
decl_isr isr_irq9
decl_isr isr_irq10
decl_isr isr_irq11
decl_isr isr_irq12
decl_isr isr_irq13
decl_isr isr_irq14
decl_isr isr_irq15
decl_isr isr_irq16
decl_isr isr_irq17
decl_isr isr_irq18
decl_isr isr_irq19
decl_isr isr_irq20
decl_isr isr_irq21
decl_isr isr_irq22
decl_isr isr_irq23
decl_isr isr_irq24
decl_isr isr_irq25
decl_isr isr_irq26
decl_isr isr_irq27
decl_isr isr_irq28
decl_isr isr_irq29
decl_isr isr_irq30
decl_isr isr_irq31

// All unhandled USER IRQs fall through to here
// Gets the exeption number in R0
.global __unhandled_user_irq
.thumb_func
__unhandled_user_irq:
    bl __get_current_exception
    subs r0, #16
.global unhandled_user_irq_num_in_r0
unhandled_user_irq_num_in_r0:
    bkpt #0
    
.section .text

// Reset handler:
// - initialises .data
// - clears .bss
// - calls main
.type _reset_handler,%function
.thumb_func
_reset_handler:
    adr r4, data_cpy_table
    // assume there is at least one entry
1:
    ldmia r4!, {r1-r3}
    cmp r1, #0
    beq 2f
    bl data_cpy
    b 1b
2:
    // Zero out the BSS
    ldr r1, =__bss_start__  // Load the bss start adress into r1
    ldr r2, =__bss_end__    // Load the bss start adress into r1
    movs r0, #0
    b bss_fill_test
bss_fill_loop:
    stm r1!, {r0}           // Write the value of R0 (0) to the adress in R1, increment R1 after
bss_fill_test:
    cmp r1, r2
    bne bss_fill_loop

_entry:
    // Call the main function (C)
	ldr r1, =main
	blx r1
//	ldr r1, =exit
//    blx r1

//.weak _exit
//.type _exit,%function
//.thumb_func
//_exit:
//1: // separate label because _exit can be moved out of branch range
 //   bkpt #0
 //   b 1b



// Returns the exception number in r0. See table 2-5 in Cortex M0+ Device Generic User Guide
.global __get_current_exception
.thumb_func
__get_current_exception:
    mrs  r0, ipsr   // Read the interupt program status register into r0
    uxtb r0, r0     // Zero r0 except for the last byte
    bx   lr         // Return



data_cpy_loop:
    ldm r1!, {r0}
    stm r2!, {r0}
data_cpy:
    cmp r2, r3
    blo data_cpy_loop
    bx lr

.align 2
data_cpy_table:

.word __etext
.word __data_start__
.word __data_end__

.word __scratch_x_source__
.word __scratch_x_start__
.word __scratch_x_end__

.word __scratch_y_source__
.word __scratch_y_start__
.word __scratch_y_end__

.word 0 // null terminator

// ----------------------------------------------------------------------------
// Stack/heap dummies to set size

.section .stack
// align to allow for memory protection (although this alignment is pretty much ignored by linker script)
.align 5
    .equ StackSize, 0x800
.space StackSize

.section .heap
.align 2
    .equ HeapSize, 0x800
.space HeapSize
