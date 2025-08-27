CC=gcc
AS=nasm
LD=ld
CFLAGS=-m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -O2 -Wall -Wextra
ASFLAGS=-f elf32
LDFLAGS=-m elf_i386 -T linker.ld -nostdlib

SRC=src/kernel
BUILD=build

C_SOURCES=$(wildcard $(SRC)/*.c)
C_OBJECTS=$(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(C_SOURCES))

ASM_SOURCES=$(wildcard $(SRC)/*.asm)
ASM_OBJECTS=$(patsubst $(SRC)/%.asm,$(BUILD)/%.o,$(ASM_SOURCES))

OBJECTS=$(ASM_OBJECTS) $(C_OBJECTS)

all: mervos.iso

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/kernel.bin: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

iso/boot/kernel.bin: $(BUILD)/kernel.bin grub/grub.cfg
	mkdir -p iso/boot/grub
	cp grub/grub.cfg iso/boot/grub/grub.cfg
	cp $(BUILD)/kernel.bin iso/boot/kernel.bin

mervos.iso: iso/boot/kernel.bin
	grub-mkrescue -o mervos.iso iso

# --- GRUB only (no kernel) ---
grub-only:
	mkdir -p iso/boot/grub
	echo 'set timeout=5' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "MervOS Bootloader Only" {' >> iso/boot/grub/grub.cfg
	echo '    echo "GRUB is working!"' >> iso/boot/grub/grub.cfg
	echo '    halt' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o grub-only.iso iso

run: mervos.iso
	qemu-system-x86_64 -cdrom mervos.iso -m 256M

run-grub: grub-only
	qemu-system-x86_64 -cdrom grub-only.iso -m 256M

clean:
	rm -rf $(BUILD) mervos.iso grub-only.iso
	rm -rf iso/boot/kernel.bin
	rm -rf iso/boot/grub/grub.cfg
	rmdir --ignore-fail-on-non-empty -p iso/boot/grub iso/boot iso


.PHONY: all run clean grub-only run-grub
