# Compiler and Toolchain
CC = aarch64-linux-gnu-gcc
AS = aarch64-linux-gnu-as
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
OBJDUMP = aarch64-linux-gnu-objdump

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Linker script path
LINKER_SCRIPT_PATH = linker/linker.ld

# Automatically find source files
S_SOURCES = $(wildcard $(SRC_DIR)/*.s)
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Generate object file names from source files
# Replace $(SRC_DIR)/%.s with $(OBJ_DIR)/%.o
S_OBJS = $(patsubst $(SRC_DIR)/%.s, $(OBJ_DIR)/%.o, $(S_SOURCES))
# Replace $(SRC_DIR)/%.c with $(OBJ_DIR)/%.o
C_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_SOURCES))

OBJS = $(S_OBJS) $(C_OBJS)

# Final binaries (now in BUILD_DIR)
ELF_NAME = boot.elf
BIN_NAME = boot.bin
ELF = $(BUILD_DIR)/$(ELF_NAME)
BIN = $(BUILD_DIR)/$(BIN_NAME)

# Flags
# Common flags for C and Assembly where applicable
COMMON_FLAGS = -g -I$(INCLUDE_DIR) -mcpu=cortex-a53
CFLAGS = -Wall -O0 -std=c11 -ffreestanding -nostdlib $(COMMON_FLAGS)
ASFLAGS = $(COMMON_FLAGS)
LDFLAGS = -nostdlib -T $(LINKER_SCRIPT_PATH)

# List of directories to create
DIRS_TO_CREATE = $(BUILD_DIR) $(OBJ_DIR)

# Default target
all: $(DIRS_TO_CREATE) $(BIN)

# Rule to create directories
$(DIRS_TO_CREATE):
	mkdir -p $@

# Pattern rule for .s to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s | $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# Pattern rule for .c to .o files
# Note: This simplified pattern rule doesn't automatically handle header dependencies.
# For more robust dependency tracking, C compilers can generate .d (dependency) files.
# For this project size, a full rebuild after header changes is often acceptable.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link to ELF using linker script
$(ELF): $(OBJS) $(LINKER_SCRIPT_PATH) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Convert ELF to raw binary
$(BIN): $(ELF) | $(BUILD_DIR)
	$(OBJCOPY) -O binary $< $@

# Run in QEMU
run: $(ELF)
	timeout 3s qemu-system-aarch64 -machine virt -cpu max -m 64M -nographic -kernel $(ELF)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f uart.log

.PHONY: all clean run
