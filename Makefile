# Variables
ASM = aarch64-linux-gnu-as
LD  = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

# Files
ASM_SRC = boot.s
ELF = boot.elf
BIN = boot.bin
LINKER_SCRIPT = linker.ld

# Default target
all: $(BIN)

# Build the object file from assembly
boot.o: $(ASM_SRC)
	$(ASM) -g -o boot.o $(ASM_SRC)

# Link to ELF using linker script
$(ELF): boot.o $(LINKER_SCRIPT)
	$(LD) -o $@ -T $(LINKER_SCRIPT) boot.o

# Convert ELF to flat binary, including text and rodata, strip debug
$(BIN): $(ELF)
	# $(OBJCOPY) -O binary --only-section=.text --only-section=.rodata boot.elf boot.bin
	$(OBJCOPY) -O binary boot.elf boot.bin

# Run in QEMU (stop with Ctrl+C)
run: $(BIN)
	$(QEMU) -machine virt -cpu max -m 64M -nographic -serial file:uart.log -kernel $(BIN)

# Clean build artifacts
clean:
	rm -f *.o $(ELF) $(BIN) uart.log
