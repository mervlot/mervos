; src/kernel.asm
; small assembly helpers (currently minimal). Keep this file for future low-level helpers
; e.g., PIC/IDT setup, irq handlers, asm helpers callable from C.
[BITS 32]

SECTION .text
global asm_hang

asm_hang:
    cli
1:  hlt
    jmp 1b
