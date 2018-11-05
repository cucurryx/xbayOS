TARGET_DIR = ./build
KERNEL_ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld 
LIB = -I lib/ -I kernel/ -I device/
ASFLAGS = -f elf -I lib/include/
CFLAGS = -m32 -fno-stack-protector -fno-builtin $(LIB) -c
LDFLAGS = -m elf_i386 -Ttext $(KERNEL_ENTRY_POINT) -e main -Map $(TARGET_DIR)/kernel.map

OBJS = $(TARGET_DIR)/main.o $(TARGET_DIR)/init.o $(TARGET_DIR)/interrupt.o \
      $(TARGET_DIR)/timer.o $(TARGET_DIR)/kernel.o $(TARGET_DIR)/print.o \
      $(TARGET_DIR)/debug.o $(TARGET_DIR)/string.o $(TARGET_DIR)/bitmap.o \
	  $(TARGET_DIR)/memory.o

$(TARGET_DIR)/main.o: kernel/main.c lib/print.h lib/stdint.h kernel/init.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/init.o: kernel/init.c kernel/init.h lib/print.h lib/stdint.h kernel/interrupt.h device/timer.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h lib/stdint.h kernel/global.h lib/io.h lib/print.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h lib/io.h lib/print.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/debug.o: kernel/debug.c kernel/debug.h lib/print.h lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/string.o: lib/string.c lib/string.h lib/stdint.h kernel/debug.c kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/bitmap.o: lib/bitmap.c lib/bitmap.h lib/stdint.h kernel/debug.c kernel/debug.h 
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/memory.o: kernel/memory.c kernel/memory.h lib/bitmap.c lib/bitmap.h lib/stdint.h 
	$(CC) $(CFLAGS) $< -o $@


$(TARGET_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@
$(TARGET_DIR)/print.o: lib/print.S
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY: clean all hd

clean:
	rm -rf $(TARGET_DIR)/*

hd: 
	dd if=/home/xvvx/xbayOS/build/kernel.bin of=/home/xvvx/xbayOS/bochs/bin/hd60m.img bs=512 count=200 seek=9 conv=notrunc

build: $(TARGET_DIR)/kernel.bin

all: build hd
