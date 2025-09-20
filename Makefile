BUILD=build
KERNEL_SRC=src/kernel
ISO_DIR=iso

all: mervos.iso

# --- Copy prebuilt kernel and GRUB ---
iso/boot/kernel.bin: $(KERNEL_SRC)/kernel.bin grub/grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp $(KERNEL_SRC)/kernel.bin $(ISO_DIR)/boot/kernel.bin

mervos.iso: iso/boot/kernel.bin
	grub-mkrescue -o mervos.iso $(ISO_DIR)

# --- GRUB only (no kernel) ---
grub-only:
	mkdir -p $(ISO_DIR)/boot/grub
	echo 'set timeout=5' > $(ISO_DIR)/boot/grub/grub.cfg
	echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo 'menuentry "MervOS Bootloader Only" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '    echo "GRUB is working!"' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '    halt' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o grub-only.iso $(ISO_DIR)

# --- Run ---
run: mervos.iso
	qemu-system-x86_64 -cdrom mervos.iso -m 256M

run-grub: grub-only
	qemu-system-x86_64 -cdrom grub-only.iso -m 256M

# --- Clean ---
clean:
	rm -rf $(BUILD) mervos.iso grub-only.iso
	rm -rf $(ISO_DIR)/boot/kernel.bin
	rm -rf $(ISO_DIR)/boot/grub/grub.cfg
	rmdir --ignore-fail-on-non-empty -p $(ISO_DIR)/boot/grub $(ISO_DIR)/boot $(ISO_DIR)

.PHONY: all run clean grub-only run-grub