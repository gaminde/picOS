# Variables
ASM = aarch64-linux-gnu-as
CC  = aarch64-linux-gnu-gcc # C compiler
LD  = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

# Files
BOOT_SRC = boot.s
KERNEL_C_SRC = kernel.c # C source for kernel
OBJS = boot.o kernel.o
ELF = boot.elf
BIN = boot.bin
LINKER_SCRIPT = linker.ld

# CFLAGS for kernel compilation
CFLAGS = -c -ffreestanding -nostdlib -g -Wall -Wextra

# Default target
all: $(BIN)

# Build boot.o from assembly
boot.o: $(BOOT_SRC)
	$(ASM) -g -o boot.o $(BOOT_SRC)

# Build kernel.o from C
kernel.o: $(KERNEL_C_SRC)
	$(CC) $(CFLAGS) -o kernel.o $(KERNEL_C_SRC)

# Link to ELF using linker script
$(ELF): $(OBJS) $(LINKER_SCRIPT)
	$(LD) -o $@ -T $(LINKER_SCRIPT) $(OBJS)

# Convert ELF to flat binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary boot.elf boot.bin

# Run in QEMU
run: $(ELF) # Using ELF with QEMU is often better for debugging
	$(QEMU) -machine virt -cpu max -m 64M -nographic -serial file:uart.log -kernel $(ELF)

# Clean build artifacts
clean:
	rm -f *.o $(ELF) $(BIN) uart.log
