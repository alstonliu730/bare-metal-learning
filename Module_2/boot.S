.section ".text.boot" // Make sure linker puts this at the start of kernel

.global _start // Execution starts
.global __bss_start
.global __bss_end

_start:
    // Check Processor ID is 0 (main core), else hang
    mrs     x1, mpidr_el1       // mpidr_el1 = Multiprocessor ID Register
    and     x1, x1, #3
    cbz     x1, 2f

1:  
    // Not on main core, infinite wait loop
    wfe                         // wait for event instruction - power efficient
    b       1b

2:  // On main core
    // Sets the stack to start below our code
    ldr     x1, =_start
    mov     sp, x1

    // Clean the BSS section
    ldr     x1, =__bss_start    // Start address
    ldr     x2, =__bss_end      // End address

3:  // quit the loop if zero
    cmp     x1, x2
    b.ge    4f                  // Branch if start >= end
    str     xzr, [x1], #8       // Clear 8 bytes, increment x1
    b       3b                  // Loop

4:  // Jump to our main() routine in C (make sure it doesn't return)
    bl      main
    // in case it does return, halt the master core too
    b       1b
