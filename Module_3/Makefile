CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)
GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
GCC = aarch64-none-elf-gcc
LINK = aarch64-none-elf-ld
OBJCOPY = aarch64-none-elf-objcopy

all: clean kernel8.img

boot.o: boot.S
	$(GCC) $(GCCFLAGS) -c boot.S -o boot.o

%.o: %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

kernel8.img: boot.o $(OFILES)
	$(LINK) -nostdlib boot.o $(OFILES) -T link.ld -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true