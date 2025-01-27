CC := i386-elf-gcc
LD := i386-elf-gcc
AS := nasm

CCFLAGS := -std=c99 -c -ffreestanding -I./src/libk/include -I./src/include
LDFLAGS := -ffreestanding -O2 -nostdlib -lgcc

BUILDDIR 	:= ./build
SRCDIR		:= ./src

C_SRC		:= $(shell find $(SRCDIR) -name '*.c')
ASM_SRC		:= $(shell find $(SRCDIR) -name '*.asm')
C_HEADERS	:= $(shell find $(SRCDIR) -name '*.h')
C_OBJS 		:= $(C_SRC:%=$(BUILDDIR)/%.o)
ASM_OBJS	:= $(ASM_SRC:%=$(BUILDDIR)/%.o)

all: $(BUILDDIR)/kernel.iso

$(BUILDDIR)/kernel.iso: $(BUILDDIR)/kernel.bin
	mkdir -p $(BUILDDIR)/kernel/boot/grub
	cp $(SRCDIR)/grub/grub.cfg $(BUILDDIR)/kernel/boot/grub
	cp $(BUILDDIR)/kernel.bin $(BUILDDIR)/kernel/boot
	grub-mkrescue -o $(BUILDDIR)/kernel.iso $(BUILDDIR)/kernel/

$(BUILDDIR)/kernel.bin: $(ASM_OBJS) $(C_OBJS)
	$(LD) -T ./src/linker.ld -Wl,-Map=$(BUILDDIR)/kernel.map -o $@ $(LDFLAGS) $(ASM_OBJS) $(C_OBJS)

$(BUILDDIR)/%.c.o: %.c
#	@echo "CC  " $<
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ $<

$(BUILDDIR)/%.asm.o: %.asm
#	@echo "ASM " $<
	@mkdir -p $(dir $@)
	$(AS) $< -f elf32 -I./src/include/asm -o $@

.PHONY: all clean

clean:
	rm -rf ./build
	rm -rf kernel.iso