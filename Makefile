CC := i386-elf-gcc
CXX := i386-elf-g++
LD := i386-elf-gcc
AS := nasm

CCFLAGS := -std=c99 -O2 -MMD -MP -c -ffreestanding -I./src/libk/include -I./src/include
CXXFLAGS := -O2 -MMD -MP -c -ffreestanding -I./src/libk/include -I./src/include
LDFLAGS := -ffreestanding -O2 -nostdlib -lgcc

BUILDDIR 	:= ./build
SRCDIR		:= ./src

C_SRC		:= $(shell find $(SRCDIR) -name '*.c')
CPP_SRC	    := $(shell find $(SRCDIR) -name '*.cpp')
ASM_SRC		:= $(shell find $(SRCDIR) -name '*.asm')
FONT_SRC	:= $(shell find $(SRCDIR) -name '*.psf')
C_HEADERS	:= $(shell find $(SRCDIR) -name '*.h')
FONT_OBJ	:= $(FONT_SRC:%=$(BUILDDIR)/%.o)
C_OBJS 		:= $(C_SRC:%=$(BUILDDIR)/%.o)
CPP_OBJS    := $(CPP_SRC:%=$(BUILDDIR)/%.o)
ASM_OBJS	:= $(ASM_SRC:%=$(BUILDDIR)/%.o)

all: $(BUILDDIR)/kernel.iso

$(BUILDDIR)/kernel.iso: $(BUILDDIR)/kernel.bin
	@mkdir -p $(BUILDDIR)/kernel/boot/grub
	@cp $(SRCDIR)/grub/grub.cfg $(BUILDDIR)/kernel/boot/grub
	@cp $(BUILDDIR)/kernel.bin $(BUILDDIR)/kernel/boot
	@grub-mkrescue -o $(BUILDDIR)/kernel.iso $(BUILDDIR)/kernel/

$(BUILDDIR)/kernel.bin: $(ASM_OBJS) $(C_OBJS) $(CPP_OBJS) $(FONT_OBJ)
	@echo "LD       " $^
	@$(LD) -T ./src/linker.ld -Wl,-Map=$(BUILDDIR)/kernel.map -o $@ $(LDFLAGS) $(ASM_OBJS) $(C_OBJS) $(FONT_OBJ)

$(BUILDDIR)/%.c.o: %.c
	@echo "CC       " $<
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -o $@ $<

$(BUILDDIR)/%.cpp.o: %.cpp
	@echo "CXX      " $<
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) -o $@ $<

$(BUILDDIR)/%.asm.o: %.asm
	@echo "ASM      " $<
	@mkdir -p $(dir $@)
	@$(AS) -f elf32 -I./src/include/asm -o $@ $<

$(BUILDDIR)/%.psf.o: %.psf
	@echo "OBJCOPY  " $<
	@mkdir -p $(dir $@)
	@objcopy -I binary -O elf32-i386 -B i386 $< $@

.PHONY: all clean 

clean:
	rm -rf ./build

-include $(C_OBJS:.o=.d)