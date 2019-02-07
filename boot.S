/* sets up c runtime environment */
.section ".text.boot"

/* code location in compiled binary and reference name */
.global _start

_start:
    /* halts 3/4 of the ARM cores so we only deal with 1 */
    mrc p15, #0, r1, c0, c0, #5
    and r1, r1, #3
    cmp r1, #0
    bne halt

    /* our c stack should start at address 0x8000 and grow downwards */
    mov sp, #0x8000

    /* loads addresses in BSS section (uninitialized data) into registers and zeros them */
    ldr r4, =__bss_start
    ldr r9, =__bss_end
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    b       2f /* b instruction = 'do' */

1:
    stmia r4!, {r5-r8} /* store values of r5-r8 into r4, then increment 16 bytes */

2:
    cmp r4, r9
    blo 1b

    /* loads kernel_main (C function) into register3 and jumps to that location */
    ldr r3, =kernel_main
    blx r3

/* halt procedure begins and loops doing nothing */
halt:
    wfe /* while (!event_has_occurred) */
    b halt