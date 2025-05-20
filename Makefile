# Variables
ASM = aarch64-linux-gnu-as
CC  = aarch64-linux-gnu-gcc
LD  = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

# Source files
BOOT_SRC = boot.s
VECTORS_SRC = vectors.s
KERNEL_C_SRC = kernel.c

# Object files
OBJS = boot.o $(VECTORS_SRC:.s=.o) kernel.o

# Output files
ELF = boot.elf
BIN = boot.bin
LINKER_SCRIPT = linker.ld

# CFLAGS for kernel compilation
CFLAGS = -c -ffreestanding -nostdlib -g -Wall -Wextra
# ASFLAGS for assembly compilation
ASFLAGS = -g

# Default target
all: $(BIN)

# Build boot.o from assembly
boot.o: $(BOOT_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

# Build vectors.o from assembly
$(VECTORS_SRC:.s=.o): $(VECTORS_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

# Build kernel.o from C
kernel.o: $(KERNEL_C_SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Link to ELF using linker script
$(ELF): $(OBJS) $(LINKER_SCRIPT)
	$(LD) -o $@ -T $(LINKER_SCRIPT) $(OBJS)

# Convert ELF to flat binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Run in QEMU
run: $(ELF)
	$(QEMU) -machine virt -cpu max -m 64M -nographic -serial file:uart.log -kernel $(ELF)

# Clean build artifacts
clean:
	rm -f *.o $(ELF) $(BIN) uart.log
