.section ".text.boot" // Make sure linker puts this at the start of kernel

.global _start // Execution starts

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
    ldr     w2, =__bss_size     // size of the section

3:  // quit the loop if zero
    cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b              // Loop if non-zero

4:  // Jump to our main() routine in C (make sure it doesn't return)
    bl      main
    // in case it does return, halt the master core too
    b       1b
