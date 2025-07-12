.PHONY: all upload clean

PROGRAM_NAME := bpithermo
SOURCE_DIR := src
BUILD_DIR := build
PROGRAMMER_PORT := /dev/ttyACM0
CC := avr-gcc
OBJCOPY_PROG := avr-objcopy

OBJECTS := \
$(BUILD_DIR)/main.o \
$(BUILD_DIR)/hdc2080.o \
$(BUILD_DIR)/leds/leds.o \
$(BUILD_DIR)/leds/leds_asm.o \
$(BUILD_DIR)/brightness_control.o \
$(BUILD_DIR)/bat_mon.o \
$(BUILD_DIR)/eeprom.o

all: $(BUILD_DIR)/$(PROGRAM_NAME).bin

$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)
	mkdir -pv $(BUILD_DIR)/leds

# Compile C code files.
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) -mmcu=attiny48 -Wall -Wextra -Os -c $< -o $@

# Compile ASM code files.
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s | $(BUILD_DIR)
	$(CC) -mmcu=attiny48 -x assembler -Wall -Wextra -Os -c $< -o $@

# Compile ASM code files.
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S | $(BUILD_DIR)
	$(CC) -mmcu=attiny48 -Wall -Wextra -Os -c $< -o $@

# Link C code files into an ELF.
$(BUILD_DIR)/$(PROGRAM_NAME).elf: $(OBJECTS)
	$(CC) -mmcu=attiny48 $^ -o $(BUILD_DIR)/$(PROGRAM_NAME).elf

# Extract raw flash data from ELF.
$(BUILD_DIR)/$(PROGRAM_NAME).bin: $(BUILD_DIR)/$(PROGRAM_NAME).elf
	$(OBJCOPY_PROG) -j .text -j .data -O binary $(BUILD_DIR)/$(PROGRAM_NAME).elf $(BUILD_DIR)/$(PROGRAM_NAME).bin

# Upload raw flash data.
upload: $(BUILD_DIR)/$(PROGRAM_NAME).bin
	avrdude -c arduino -p attiny48 -P $(PROGRAMMER_PORT) -b 19200 -U flash:w:$(BUILD_DIR)/$(PROGRAM_NAME).bin

clean:
	rm -r $(BUILD_DIR)
