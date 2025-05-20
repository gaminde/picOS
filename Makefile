# Variables
ASM = aarch64-linux-gnu-as
LD  = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

# Files
BOOT_SRC = boot.s
KERNEL_SRC = kernel.s
OBJS = boot.o kernel.o # Add kernel.o
ELF = boot.elf
BIN = boot.bin
LINKER_SCRIPT = linker.ld

# Default target
all: $(BIN)

# Build the object file from assembly
boot.o: $(BOOT_SRC)
	$(ASM) -g -o boot.o $(BOOT_SRC)

kernel.o: $(KERNEL_SRC) # Rule to build kernel.o
	$(ASM) -g -o kernel.o $(KERNEL_SRC)

# Link to ELF using linker script
$(ELF): $(OBJS) $(LINKER_SCRIPT) # Depend on all OBJS
	$(LD) -o $@ -T $(LINKER_SCRIPT) $(OBJS) # Link all OBJS

# Convert ELF to flat binary, including text and rodata, strip debug
$(BIN): $(ELF)
	# $(OBJCOPY) -O binary --only-section=.text --only-section=.rodata boot.elf boot.bin
	$(OBJCOPY) -O binary boot.elf boot.bin

# Run in QEMU (stop with Ctrl+C)
run: $(BIN)
	$(QEMU) -machine virt -cpu max -m 64M -nographic -kernel $(ELF)

# Clean build artifacts
clean:
	rm -f *.o $(ELF) $(BIN) uart.log
