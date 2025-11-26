CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -O0 -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ISO_DIR = isodir
KERNEL = kernel.bin
ISO = my_kernel.iso
QEMU = qemu-system-x86_64

# فایل‌های ماژول‌ها
MEMORY_MANAGER = kernel/memory_manager/memory_manager.o

# کامپایل ماژول memory_manager
kernel/memory_manager/memory_manager.o: kernel/memory_manager/memory_manager.c
	$(CC) $(CFLAGS) -c $< -o $@

# کرنل اصلی
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@
	g++ ramfs.cpp -o ramfs_creator

# لینک کردن کرنل و ماژول‌ها
$(KERNEL): kernel.o $(MEMORY_MANAGER)
	$(LD) $(LDFLAGS) $^ -o $@

# تنظیم ISO
setup_iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	./ramfs_creator
	cp kernel/memory_manager/memory_manager.o $(ISO_DIR)/boot/  # کپی ماژول به پوشه ISO
	echo 'menuentry "SINUX Project" { multiboot /boot/kernel.bin; module /boot/ramfs; module /boot/memory_manager.o; boot; }' > $(ISO_DIR)/boot/grub/grub.cfg

# ساخت ISO
$(ISO): setup_iso
	grub-mkrescue -o $@ $(ISO_DIR)

# اجرای پروژه
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

# پاکسازی فایل‌های موقت
clean:
	rm -rf *.o $(KERNEL) $(ISO) $(ISO_DIR)

.PHONY: all run clean