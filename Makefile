CC := clang #i386-linux-gcc
LD := ld.lld
ASM := nasm
CFLAGS := -std=c99 -ffreestanding -nostdlib -c -I./src/kernel -I./src/libs
CCFLAGS := -std=c99 -c -ffreestanding -target i686-none-elf -I./src/kernel -I./src/libs
LINKFLAGS := -L. -nostdlib --oformat binary


MODULES := stage1 stage2 kernel
BUILD_DIR:= ./build
STAGE1_DIR := ./src/stage1
STAGE2_DIR := ./src/stage2
KERNEL_DIR := ./src/kernel

STAGE1_SRC :=  $(shell find $(STAGE1_DIR) -name '*.asm')

STAGE2_SRC :=  $(shell find $(STAGE2_DIR) -name '*.asm' -or -name '*.c')
STAGE2_OBJS := $(STAGE2_SRC:%=$(BUILD_DIR)/%.o)

KERNEL_SRC := $(shell find $(KERNEL_DIR) -name '*.asm' -or -name '*.c')
KERNEL_OBJS := $(KERNEL_SRC:%=$(BUILD_DIR)/%.o)

BOOTLOADER_STAGE1_SRC_DIR := src/bootloader/stage1

BIN_FILES := $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/stage2.bin $(BUILD_DIR)/stage1.bin

.PHONY: all clean

$(BUILD_DIR)/main_floppy.img: $(BIN_FILES)
	@dd if=/dev/zero of=$@ bs=512 count=2880 >/dev/null 2>&1
	@mkfs.fat -F 12 -n "NBOS" $@ > /dev/null
	@dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc >/dev/null 2>&1
	@mcopy -i $@ $(BUILD_DIR)/stage2.bin "::stage2.bin"
	@mcopy -i $@ $(BUILD_DIR)/kernel.bin "::kernel.bin"
	@echo "Building floppy image: " $@

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS)
	@echo "LD  " $@
	@$(LD) -T $(KERNEL_DIR)/linker.ld $(LINKFLAGS) -Map=$(BUILD_DIR)/kernel.map -o $@ $(KERNEL_OBJS) -lgcc

$(BUILD_DIR)/stage1.bin: $(STAGE1_SRC)
	@echo "ASM " $<
	@mkdir -p $(dir $@)
	@$(ASM) $< -f bin -o $@

$(BUILD_DIR)/stage2.bin: $(STAGE2_OBJS)
	@echo "LD  " $@
	@$(LD) -T $(STAGE2_DIR)/linker.ld $(LINKFLAGS) -Map=$(BUILD_DIR)/stage2.map -o $@ $(STAGE2_OBJS) -lgcc

$(BUILD_DIR)/%.c.o: %.c
	@echo "CC  " $<
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -o $@ $<

$(BUILD_DIR)/%.asm.o: %.asm
	@echo "ASM " $<
	@mkdir -p $(dir $@)
	@$(ASM) $< -f elf -o $@
clean:
	rm -rf build