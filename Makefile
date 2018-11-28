TARGET_DIR = ./build
KERNEL_ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld 
LIB = -I lib/ -I kernel/ -I device/ -I thread/ -I proc/ 
ASFLAGS = -f elf -I lib/include/
CFLAGS = -Wall -Wmissing-prototypes -m32 -fno-stack-protector -fno-builtin $(LIB) -c
LDFLAGS = -m elf_i386 -Ttext $(KERNEL_ENTRY_POINT) -e main -Map $(TARGET_DIR)/kernel.map

OBJS = $(TARGET_DIR)/main.o $(TARGET_DIR)/init.o $(TARGET_DIR)/interrupt.o \
      $(TARGET_DIR)/timer.o $(TARGET_DIR)/kernel.o $(TARGET_DIR)/print.o \
      $(TARGET_DIR)/debug.o $(TARGET_DIR)/string.o $(TARGET_DIR)/bitmap.o \
	  $(TARGET_DIR)/memory.o $(TARGET_DIR)/list.o $(TARGET_DIR)/thread.o	\
	  $(TARGET_DIR)/switch.o $(TARGET_DIR)/lock.o $(TARGET_DIR)/console.o \
	  $(TARGET_DIR)/keyboard.o $(TARGET_DIR)/io_queue.o $(TARGET_DIR)/tss.o \
	  $(TARGET_DIR)/process.o $(TARGET_DIR)/syscall.o $(TARGET_DIR)/syscall_init.o \
	  $(TARGET_DIR)/stdio.o $(TARGET_DIR)/printk.o $(TARGET_DIR)/disk.o	\
	  $(TARGET_DIR)/dir.o $(TARGET_DIR)/file.o $(TARGET_DIR)/fs.o $(TARGET_DIR)/superblock.o

$(TARGET_DIR)/main.o: kernel/main.c lib/print.h lib/stdint.h kernel/init.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/init.o: kernel/init.c kernel/init.h lib/print.h lib/stdint.h kernel/interrupt.h device/timer.h proc/tss.h
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

$(TARGET_DIR)/thread.o: thread/thread.c thread/thread.h lib/stdint.h \
        kernel/global.h lib/bitmap.h kernel/memory.h lib/string.h \
        lib/stdint.h lib/print.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/list.o: lib/list.c lib/list.h lib/stdint.h kernel/interrupt.h kernel/interrupt.c 
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/lock.o: thread/lock.c thread/lock.h lib/stdint.h kernel/interrupt.c kernel/interrupt.h thread/thread.h thread/thread.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/console.o: device/console.c device/console.h thread/lock.c thread/lock.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/keyboard.o: device/keyboard.c device/keyboard.c 
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/io_queue.o: device/io_queue.c device/io_queue.h kernel/debug.h kernel/debug.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/tss.o: proc/tss.c proc/tss.h
	$(CC) $(CFLAGS) $< -o $@
	
$(TARGET_DIR)/process.o: proc/process.c proc/process.h 	
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/syscall.o: lib/syscall.c lib/syscall.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/syscall_init.o: proc/syscall_init.c proc/syscall_init.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/stdio.o: lib/stdio.c lib/stdio.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/printk.o: lib/printk.c lib/printk.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/disk.o: device/disk.c device/disk.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/dir.o: fs/dir.c fs/dir.h 
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/file.o: fs/file.c fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/fs.o: fs/fs.c fs/fs.h
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_DIR)/superblock.o fs/superblock.c fs/superblock.h
	$(CC) $(CFLAGS) $< -o $@






$(TARGET_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@
$(TARGET_DIR)/print.o: lib/print.S
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET_DIR)/switch.o: thread/switch.S 
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
