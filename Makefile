CFLAGS = -m32 -fno-stack-protector -fno-builtin -I lib/ -I device/ -I kernel/ -c
SOURCES = kernel/*.c device/*.c
OBJECTS = build/*.o
CC = gcc

print.o: lib/print.S
	nasm -f elf -o build/print.o lib/print.S -I lib/include/ 

kernel.o: kernel/kernel.S
	nasm -f elf -o build/kernel.o kernel/kernel.S

main.o: kernel/main.c init.o
	$(CC) $(CFLAGS) kernel/main.c -o build/main.o

interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) kernel/interrupt.c -o build/interrupt.o

init.o: kernel/init.c timer.o interrupt.o
	$(CC) $(CFLAGS) kernel/init.c -o build/init.o

timer.o: device/timer.c
	$(CC) $(CFLAGS) device/timer.c -o build/timer.o

kernel.bin: print.o kernel.o main.o interrupt.o init.o timer.o 
	ld -m elf_i386 -Ttext 0xc0001500 -e main -o kernel.bin $(OBJECTS)

all: kernel.bin
	dd if=build/kernel.bin of=bochs/bin/hd60m.img bs=512 count=200 seek=9 conv=notrunc
	bochs/bin/bochs -f bochs/bin/bochsrc.txt
