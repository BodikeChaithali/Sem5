CROSS = riscv64-unknown-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS = -march=rv64gc -mabi=lp64 -mcmodel=medany -ffreestanding -fno-pie -nostdlib -O2 -Wall
LDFLAGS = -T linker.ld -m elf64lriscv

OBJS = boot.o shell.o uart.o

all: shell.elf

boot.o: boot.S
	$(CC) $(CFLAGS) -c boot.S -o boot.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

shell.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o shell.elf $(OBJS)

run: shell.elf
	qemu-system-riscv64 -M virt -bios none -kernel shell.elf -nographic

clean:
	rm -f *.o shell.elf
