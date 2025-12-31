BOOT_SRC = boot
BUILD_SRC = build
SRC_DIR	= src
INCLUDE_DIR = -I include

CFILES = $(wildcard $(SRC_DIR)/*.c)
SFILES = $(wildcard $(BOOT_SRC)/*.S)
OFILES = $(CFILES:$(SRC_DIR)/%.c=$(BUILD_SRC)/%.o) $(SFILES:$(BOOT_SRC)/%.S=$(BUILD_SRC)/%.o)

GCCFLAGS = $(INCLUDE_DIR) -Wall -Wextra -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -mstrict-align
GCC = aarch64-none-elf-gcc
LINK = aarch64-none-elf-ld
OBJCOPY = aarch64-none-elf-objcopy

all: clean kernel8.img

$(BUILD_SRC)/boot.o: $(BOOT_SRC)/boot.S
	$(GCC) $(GCCFLAGS) -c $(BOOT_SRC)/boot.S -o $(BUILD_SRC)/boot.o

$(BUILD_SRC)/%.o: $(SRC_DIR)/%.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

$(BUILD_SRC)/%.o: $(BOOT_SRC)/%.S
	$(GCC) $(GCCFLAGS) -c $< -o $@

kernel8.img: $(BOOT_SRC)/link.ld $(OFILES)
	$(LINK) -nostdlib $(OFILES) -T $(BOOT_SRC)/link.ld -o $(BUILD_SRC)/kernel8.elf
	$(OBJCOPY) -O binary $(BUILD_SRC)/kernel8.elf kernel8.img

clean:
	rm -f $(BUILD_SRC)/kernel8.elf $(BUILD_SRC)/*.o *.img > /dev/null 2> /dev/null || true