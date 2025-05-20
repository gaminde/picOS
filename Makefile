# Variables
ASM = aarch64-linux-gnu-as
CC  = aarch64-linux-gnu-gcc
LD  = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build # Optional: for object files

# Source files
BOOT_S_SRC = $(SRC_DIR)/boot.s
VECTORS_S_SRC = $(SRC_DIR)/vectors.s
UART_S_SRC = $(SRC_DIR)/uart.s # New UART assembly source
KERNEL_C_SRC = $(SRC_DIR)/kernel.c

# Object files (consider putting them in BUILD_DIR)
# For simplicity, let's keep them in root for now, or use $(BUILD_DIR)/
BOOT_S_OBJ = boot.o # $(BOOT_S_SRC:.s=.o) would be src/boot.o
VECTORS_S_OBJ = vectors.o
UART_S_OBJ = uart.o
KERNEL_C_OBJ = kernel.o

OBJS = $(BOOT_S_OBJ) $(VECTORS_S_OBJ) $(UART_S_OBJ) $(KERNEL_C_OBJ)

# Output files
ELF = boot.elf
BIN = boot.bin
LINKER_SCRIPT = linker.ld

# Compiler and Assembler Flags
CFLAGS = -c -ffreestanding -nostdlib -g -Wall -Wextra -I$(INCLUDE_DIR) # Added -I for include path
ASFLAGS = -g

# Default target
all: $(BIN)

# Build rules
$(BOOT_S_OBJ): $(BOOT_S_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

$(VECTORS_S_OBJ): $(VECTORS_S_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

$(UART_S_OBJ): $(UART_S_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

$(KERNEL_C_OBJ): $(KERNEL_C_SRC)
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
	rm -f *.o $(ELF) $(BIN) uart.log # Also clean $(BUILD_DIR)/*.o if using BUILD_DIR
