#ifndef __DEVICE_DISK_H
#define __DEVICE_DISK_H

#include <stdint.h>
#include <list.h>
#include <bitmap.h>
#include <lock.h>

#define NAME_LEN 8
#define PRIM_PARTS_CNT 4
#define LOGIC_PARTS_CNT 8
#define CHANNEL_DEVICE_CNT 2

typedef struct __partition partition;
typedef struct __disk disk;
typedef struct __ide_channel ide_channel;
typedef struct __super_block super_block;

struct __super_block {

};


struct __partition {
    uint32_t start;
    uint32_t count;
    char name[NAME_LEN];
    disk *belong_disk;
    list_node part_tag;
    super_block *sb;
    bitmap block_bm;
    bitmap inode_bm;
    list open_inodes;
};

struct __disk {
    char name[NAME_LEN];
    ide_channel *channel;
    uint8_t no;
    partition prim_parts[PRIM_PARTS_CNT];
    partition logic_parts[LOGIC_PARTS_CNT];
};

struct __ide_channel {
    sem_t sem;
    mutex_t mutex;
    uint16_t start_port;
    uint8_t irq_no;
    bool waiting_intr;
    char name[NAME_LEN];
    disk devices[CHANNEL_DEVICE_CNT];
};

void ide_channel_init();
void ide_read(disk *d, uint32_t lba, void *buf, uint32_t cnt);
void ide_write(disk *d, uint32_t lba, void *buf, uint32_t cnt);
void disk_intr_handler(uint8_t irq);

#endif // !__DEVICE          _DISK_H