# Variables
ASM = aarch64-linux-gnu-as
CC  = aarch64-linux-gnu-gcc
LD  = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

# Directories
SRC_DIR = src
INCLUDE_DIR = include

# Source files
BOOT_S_SRC = $(SRC_DIR)/boot.s
VECTORS_S_SRC = $(SRC_DIR)/vectors.s
UART_S_SRC = $(SRC_DIR)/uart.s
KERNEL_C_SRC = $(SRC_DIR)/kernel.c
EXCEPTIONS_C_SRC = $(SRC_DIR)/exceptions.c
GIC_C_SRC = $(SRC_DIR)/gic.c
TIMER_C_SRC = $(SRC_DIR)/timer.c

# Object files (consider putting them in BUILD_DIR)
# For simplicity, let's keep them in root for now, or use $(BUILD_DIR)/
BOOT_S_OBJ = boot.o # $(BOOT_S_SRC:.s=.o) would be src/boot.o
VECTORS_S_OBJ = vectors.o
UART_S_OBJ = uart.o
KERNEL_C_OBJ = kernel.o
EXCEPTIONS_C_OBJ = exceptions.o # New object file
GIC_C_OBJ = gic.o # New GIC object file
TIMER_C_OBJ = timer.o # New Timer object file

OBJS = $(BOOT_S_OBJ) $(VECTORS_S_OBJ) $(UART_S_OBJ) $(KERNEL_C_OBJ) $(EXCEPTIONS_C_OBJ) $(GIC_C_OBJ) $(TIMER_C_OBJ)

# Output files
ELF = boot.elf
BIN = boot.bin
LINKER_SCRIPT = linker.ld

# Compiler and Assembler Flags
CFLAGS = -Wall -O0 -g -std=c11 -ffreestanding -nostdlib -I$(INCLUDE_DIR) -mcpu=cortex-a53
ASFLAGS = -g -I$(INCLUDE_DIR) -mcpu=cortex-a53
LDFLAGS = -nostdlib -T linker.ld
LINKER_SCRIPT = linker.ld

# Default target
all: $(BIN)

# Build rules
$(BOOT_S_OBJ): $(BOOT_S_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

$(VECTORS_S_OBJ): $(VECTORS_S_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

$(UART_S_OBJ): $(UART_S_SRC)
	$(ASM) $(ASFLAGS) -o $@ $<

$(KERNEL_C_OBJ): $(KERNEL_C_SRC) $(INCLUDE_DIR)/kernel.h $(INCLUDE_DIR)/uart.h $(INCLUDE_DIR)/gic.h $(INCLUDE_DIR)/timer.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXCEPTIONS_C_OBJ): $(EXCEPTIONS_C_SRC) $(INCLUDE_DIR)/exceptions.h $(INCLUDE_DIR)/uart.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(GIC_C_OBJ): $(GIC_C_SRC) $(INCLUDE_DIR)/gic.h $(INCLUDE_DIR)/uart.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(TIMER_C_OBJ): $(TIMER_C_SRC) $(INCLUDE_DIR)/timer.h $(INCLUDE_DIR)/gic.h $(INCLUDE_DIR)/uart.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Link to ELF using linker script
$(ELF): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Convert ELF to flat binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Run in QEMU
run: $(ELF)
	timeout 3s qemu-system-aarch64 -machine virt -cpu max -m 64M -nographic -kernel $(ELF)

# Clean build artifacts
clean:
	rm -f *.o $(ELF) $(BIN) uart.log
