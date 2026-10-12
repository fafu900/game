CC      = gcc -m32
NASM    = nasm
LD      = $(CC)

CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
          -I kernel -I drivers \
          -fno-stack-protector -fno-pie -nostdlib

BUILD   = build

CSRCS   = kernel/kmain.c kernel/vga_text.c kernel/idt.c kernel/kheap.c kernel/shell.c \
          drivers/keyboard.c drivers/timer.c fs/ramfs.c
ASMSRCS = boot/boot.asm kernel/entry.asm kernel/isr_stub.asm

OBJS    = $(patsubst %.c,$(BUILD)/%.o,$(CSRCS)) $(patsubst %.asm,$(BUILD)/%.o,$(filter kernel/%,$(ASMSRCS)))

all: $(BUILD)/myos.bin $(BUILD)/myos.img

$(BUILD)/boot/boot.o: boot/boot.asm
	@mkdir -p $(dir $@)
	$(NASM) -f bin -o $@ $<

$(BUILD)/kernel/%.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 -o $@ $<

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# линкуем с entry.asm первым и точкой входа kernel_entry, текст с 0x10000
$(BUILD)/kernel.elf: $(OBJS)
	$(LD) -Ttext 0x10000 -e kernel_entry -o $@ $(OBJS)

$(BUILD)/myos.bin: $(BUILD)/kernel.elf
	objcopy -I elf32-i386 -O binary $< $@

# образ диска: MBR + ядро (выровненное по секторам), без FAT — загрузчик читает LBA напрямую
$(BUILD)/myos.img: $(BUILD)/boot/boot.o $(BUILD)/myos.bin
	@mkdir -p $(BUILD)
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	dd if=$(BUILD)/boot/boot.o of=$@ bs=512 count=1 conv=notrunc status=none
	dd if=$(BUILD)/myos.bin of=$@ bs=512 seek=1 conv=notrunc status=none

run: $(BUILD)/myos.img
	qemu-system-i386 -fda $(BUILD)/myos.img -serial stdio -no-reboot

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
