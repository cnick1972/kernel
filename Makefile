LD				:= i386-elf-ld
CXX				:= i386-elf-g++
ASM				:= /usr/bin/nasm
BOOTSECT_DIR 	:= bootsect
STAGE1_DIR 		:= stage1
KERNEL_DIR 		:= kernel

BUILD_DIR		:= build
KERNEL_SRC_DIR	:= $(KERNEL_DIR)/src
KERNEL_ASM_SRC_DIR := $(KERNEL_SRC_DIR)/asm
KERNEL_SRCS		:= $(shell find $(KERNEL_SRC_DIR) -name '*.cpp')
KERNEL_ASM_SRCS := $(shell find $(KERNEL_SRC_DIR) -name '*.asm')
KERNEL_LIB_DIR	:= $(KERNEL_DIR)/Lib
KERNEL_LIB_SRC_DIR := $(KERNEL_LIB_DIR)/src
KERNEL_LIB_SRCS := $(shell find $(KERNEL_LIB_SRC_DIR) -name '*.cpp')
KERNEL_OBJS		:= $(KERNEL_ASM_SRCS:%=$(BUILD_DIR)/%.o) $(KERNEL_SRCS:%=$(BUILD_DIR)/%.o) $(KERNEL_LIB_SRCS:%=$(BUILD_DIR)/%.o)

disk1.img: $(BOOTSECT_DIR)/bootsect.bin $(STAGE1_DIR)/KRNLDR.SYS $(KERNEL_DIR)/KRNL.SYS
	@echo "[.......Creating Disk........]"
	dd if=/dev/zero of=disk1.img count=1440 bs=1k
	@echo "[.....Copying Bootsector.....]"
	dd if=$(BOOTSECT_DIR)/bootsect.bin of=disk1.img conv=notrunc
	@echo "[....Copying Kernel Loader...]"
	mcopy -i disk1.img $(STAGE1_DIR)/KRNLDR.SYS ::/
	@echo "[.......Copying Kernel.......]"
	mcopy -i disk1.img $(KERNEL_DIR)/KRNL.SYS ::/

#Build the bootsector code
$(BOOTSECT_DIR)/bootsect.bin: $(BOOTSECT_DIR)/bootsect.asm
	$(ASM) -f bin $< -o $@

#Build the stage 1 kernel loader code
$(STAGE1_DIR)/KRNLDR.SYS: $(STAGE1_DIR)/stage1.asm
	$(ASM) -f bin -i stage1/ $< -o $@

#Link the main kernel
$(KERNEL_DIR)/KRNL.SYS: $(KERNEL_OBJS)
	$(LD) -T $(KERNEL_DIR)/kernel.ld $(KERNEL_OBJS) -o $@

#Build the kernel C++ sources
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -I $(KERNEL_DIR)/Lib/Include -I $(KERNEL_DIR)/include -c $< -o $@

#Build the kernel asm sources
$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf -i $(KERNEL_ASM_SRC_DIR)/include/ $< -o $@

.PHONY: clean

clean:
	rm -fv $(BOOTSECT_DIR)/bootsect.bin
	rm -fv $(STAGE1_DIR)/KRNLDR.SYS
	rm -fv $(KERNEL_DIR)/KRNL.SYS
	rm -rfv $(BUILD_DIR)
	rm -fv disk1.img
