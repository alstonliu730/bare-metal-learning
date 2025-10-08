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
    cbz     x0, _start_el2

err_hang:  
    // Infinite wait loop (All other cores are left to hang)
    wfe                         // wait for event instruction - power efficient
    b       err_hang

_start_el2:
    // Configure EL1 execution state
    mov     x0, #0x3C5          // SPRSR_EL2: EL1h, D, A, I, F masked initially
    msr     SPSR_EL2, x0

    // Set Return address for EL1
    adr     x0, start_el1
    msr     ELR_EL2, x0

    // Configure the Hyperviser Configuration Register
    mov     x0, #(1 << 31)      // RW=1  
    // orr     x0, x0, #(1 << 4)   // IMO=1
    // orr     x0, x0, #(1 << 5)   // FMO=1
    msr     HCR_EL2, x0

    // Configure the System Control Register for EL1
    mov     x0, #0x0            // Start with MMU disabled
    msr     SCTLR_EL1, x0

    // Configure Counter-Timer Hypervisor Control
    mrs     x0, CNTHCTL_EL2     // Enables EL1 physical timers
    orr     x0, x0, #3
    msr     CNTHCTL_EL2, x0

    // Setup vector table base for EL1 (Not implemented yet)
    adr     x0, vector_table
    msr     VBAR_EL1, x0

    // Clears the Virtualization Processor ID Registers allowing our own virtual CPU ID info
    msr     VMPIDR_EL2, xzr
    msr     VPIDR_EL2, xzr

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
    // Unmask all interrupts
    bl      irq_enable
    // Jump to our main() routine in C (make sure it doesn't return)
    bl      main
    // in case it does return, halt the master core too
    b       err_hang
