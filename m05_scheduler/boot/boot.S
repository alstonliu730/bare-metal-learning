#include "sysreg.h"

.section ".text.boot" // Make sure linker puts this at the start of kernel

.globl __bss_start
.globl __bss_end
.globl err_hang

.globl _start // Execution starts
_start:
    // Check Processor ID is 0 (main core), else hang
    mrs     x0, MPIDR_EL1       // mpidr_el1 = Multiprocessor ID Register
    and     x0, x0, #3
    cbz     x0, _start_el3

err_hang:  
    // Infinite wait loop (All other cores are left to hang)
    wfe                         // wait for event instruction - power efficient
    b       err_hang

_start_el3:
    // Configure the Secure Status of the lower exception levels
    ldr     x0, =SCR_VALUE
    msr     SCR_EL3, x0

    // Configure EL1 execution state
    mov     x0, #0x3C5          // SPRSR_EL2: EL1h, D, A, I, F masked initially
    msr     SPSR_EL3, x0

    // Set Return address for EL1
    adr     x0, start_el1
    msr     ELR_EL3, x0

    // Configure the Hyperviser Configuration Register
    ldr     x0, =HCR_RW
    msr     HCR_EL2, x0

    // Configure the System Control Register for EL1
    ldr     x0, =SCTLR_VALUE_MMU_DISABLED
    msr     SCTLR_EL1, x0

    // Setup vector table base for EL1 (Not implemented yet)
    ldr     x0, =vector_table
    msr     VBAR_EL1, x0

    eret

start_el1:  // On main core
    // Sets the stack to start below our code
    ldr     x0, =_start
    mov     sp, x0

    // Clean the BSS section
    ldr     x0, =__bss_start    // Start address
    ldr     x1, =__bss_end      // End address

3:  // quit the loop if zero
    cmp     x0, x1
    b.ge    4f                  // Branch if start >= end
    str     xzr, [x0], #8       // Clear 8 bytes, increment x1
    b       3b                  // Loop

4:  
    // Jump to our main() routine in C (make sure it doesn't return)
    bl      main
    // in case it does return, halt the master core too
    b       err_hang
