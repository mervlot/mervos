#!/usr/bin/env bash
set -euo pipefail

echo "[*] Creating MervOS (GRUB + 32-bit) skeleton..."

# Folders
mkdir -p src iso/boot/grub build

# --- linker.ld ---
cat > linker.ld <<'LD'
ENTRY(_start)
SECTIONS
{
  . = 1M;
  .multiboot : { *(.multiboot) }
  .text      : { *(.text*) }
  .rodata    : { *(.rodata*) }
  .data      : { *(.data*) }
  .bss       : { *(.bss*) *(COMMON) }
}
LD

# --- src/kernel_entry.asm ---
cat > src/kernel_entry.asm <<'ASM'
; Multiboot v1 + 32-bit entry stub for GRUB
[BITS 32]

SECTION .multiboot
align 4
    dd 0x1BADB002                 ; magic
    dd 0x00010003                 ; flags: mem info | video
    dd -(0x1BADB002 + 0x00010003) ; checksum

SECTION .bss
align 16
stack_bottom:
    resb 16384
stack_top:

SECTION .text
global _start
extern kmain

_start:
    cli
    mov esp, stack_top
    xor ebp, ebp
    ; Multiboot passes:
    ;   EAX = magic (0x2BADB002)
    ;   EBX = pointer to multiboot info
    push ebx          ; arg1 = mbi
    push eax          ; arg0 = magic
    call kmain
.hang:
    cli
    hlt
    jmp .hang
ASM

# --- src/kernel.c ---
cat > src/kernel.c <<'C'
#include <stdint.h>
#include <stddef.h>

/* ======= Basic port I/O (inline asm) ======= */
static inline uint8_t inb(uint16_t port) {
  uint8_t val;
  __asm__ __volatile__("inb %1, %0" : "=a"(val) : "Nd"(port));
  return val;
}
static inline void outb(uint16_t port, uint8_t val) {
  __asm__ __volatile__("outb %0, %1" :: "a"(val), "Nd"(port));
}

/* ======= VGA text mode ======= */
#define VGA_MEM ((volatile uint16_t*)0xB8000)
#define VGA_COLS 80
#define VGA_ROWS 25
static size_t cursor = 0;
static uint8_t vga_attr = 0x07; // light grey on black

static void vga_putc(char c) {
  if (c == '\n') {
    cursor += (VGA_COLS - (cursor % VGA_COLS));
  } else {
    VGA_MEM[cursor++] = (uint16_t)c | ((uint16_t)vga_attr << 8);
  }
  if (cursor >= VGA_COLS * VGA_ROWS) cursor = 0; // naive wrap
}
static void vga_print(const char* s) { while (*s) vga_putc(*s++); }
static void vga_clear(void){
  for (size_t i=0;i<VGA_COLS*VGA_ROWS;i++) VGA_MEM[i] = ((uint16_t)vga_attr<<8) | ' ';
  cursor = 0;
}

/* ======= Tiny libc-free helpers ======= */
static int streq(const char* a, const char* b){
  while(*a && (*a==*b)){ a++; b++; }
  return (*a==0 && *b==0);
}
static int starts_with(const char* s, const char* p){
  while(*p){ if(*s++ != *p++) return 0; }
  return 1;
}
static size_t strlen_(const char* s){ size_t n=0; while(*s++) n++; return n; }

/* ======= Keyboard (PS/2 polling, set 1) ======= */
#define PS2_STAT 0x64
#define PS2_DATA 0x60

static const char sc_to_ascii[128] = {
  0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
  '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
  'a','s','d','f','g','h','j','k','l',';','\'','`',  0,'\\',
  'z','x','c','v','b','n','m',',','.','/', 0,'*',  0,' ', 0,
  /* rest zeros */
};

static int kb_has_data(void){ return (inb(PS2_STAT) & 1); }
static uint8_t kb_read_sc(void){ return inb(PS2_DATA); }

/* ======= Shell ======= */
#define LINE_MAX 128
static char line[LINE_MAX];

static void print_prompt(void){ vga_print("mervos> "); }

static void handle_cmd(const char* s){
  while(*s==' ') s++;               // trim left
  if (*s==0) return;
  if (streq(s,"help")){
    vga_print("Commands:\n  help\n  cls\n  echo <text>\n");
  } else if (streq(s,"cls")){
    vga_clear();
  } else if (starts_with(s,"echo ")){
    vga_print(s+5); vga_putc('\n');
  } else {
    vga_print("Unknown. Try 'help'.\n");
  }
}

static void read_line_and_dispatch(void){
  size_t idx=0;
  for(;;){
    // wait for a make scancode (<0x80)
    uint8_t sc;
    do { while(!kb_has_data()) { /* spin */ } sc = kb_read_sc(); } while(sc & 0x80);

    char c = 0;
    if (sc < 128) c = sc_to_ascii[sc];

    if (c == 0) continue;
    if (c == '\n'){            // ENTER
      vga_putc('\n');
      line[idx] = 0;
      handle_cmd(line);
      break;
    } else if (c == '\b'){     // BACKSPACE
      if (idx > 0) {
        idx--;
        // move cursor back one: naive – print backspace effect
        if (cursor > 0) cursor--;
        vga_putc(' ');
        if (cursor > 0) cursor--;
      }
    } else {                   // regular char
      if (idx < LINE_MAX-1){
        line[idx++] = c;
        vga_putc(c);
      }
    }
  }
}

/* ======= Kernel entry from ASM ======= */
void kmain(uint32_t magic, uint32_t mbi){
  (void)magic; (void)mbi;
  vga_clear();
  vga_print("MervOS - 32-bit (GRUB, polling keyboard)\n");
  for(;;){
    print_prompt();
    read_line_and_dispatch();
  }
}
C

# --- grub/grub.cfg (source copy) ---
cat > grub/grub.cfg <<'CFG'
set timeout=0
set default=0
menuentry "MervOS 32-bit (GRUB)" {
  multiboot /boot/kernel.bin
  boot
}
CFG

# --- Makefile ---
cat > Makefile <<'MK'
CC=gcc
AS=nasm
LD=ld
CFLAGS=-m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -O2 -Wall -Wextra
ASFLAGS=-f elf32
LDFLAGS=-m elf_i386 -T linker.ld -nostdlib

BUILD=build

all: mervos.iso

$(BUILD):
\tmkdir -p $(BUILD)

$(BUILD)/kernel_entry.o: src/kernel_entry.asm | $(BUILD)
\t$(AS) $(ASFLAGS) src/kernel_entry.asm -o $@

$(BUILD)/kernel.o: src/kernel.c | $(BUILD)
\t$(CC) $(CFLAGS) -c src/kernel.c -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel_entry.o $(BUILD)/kernel.o linker.ld
\t$(LD) $(LDFLAGS) -o $@ $(BUILD)/kernel_entry.o $(BUILD)/kernel.o

iso/boot/kernel.bin: $(BUILD)/kernel.bin grub/grub.cfg
\tmkdir -p iso/boot/grub
\tcp grub/grub.cfg iso/boot/grub/grub.cfg
\tcp $(BUILD)/kernel.bin iso/boot/kernel.bin

mervos.iso: iso/boot/kernel.bin
\tgrub-mkrescue -o mervos.iso iso

run: mervos.iso
\tqemu-system-x86_64 -cdrom mervos.iso -m 256M

clean:
\trm -rf $(BUILD) iso mervos.iso

.PHONY: all run clean
MK

echo "[*] Done."
echo "Next:"
echo "  1) make        # builds kernel + ISO"
echo "  2) make run    # boots in QEMU"
