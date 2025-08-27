/* linker.ld */
ENTRY(start)

SECTIONS {
    . = 1M;  /* Load at 1MB */

    .multiboot :
    {
        *(.multiboot)
    }

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss*)
    }
}
