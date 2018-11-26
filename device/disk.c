#include <disk.h>
#include <global.h>
#include <io.h>
#include <debug.h>
#include <timer.h>
#include <stdio.h>
#include <printk.h>
#include <interrupt.h>

//获取channel对应的IO port
#define RW_DATA(channel)             (channel->start_port + 0x0)
#define R_ERROR_W_FEATURES(channel)  (channel->start_port + 0x1)
#define RW_SECTOR_CNT(channel)       (channel->start_port + 0x2)
#define RW_LBA_LOW(channel)          (channel->start_port + 0x3)
#define RW_LBA_MID(channel)          (channel->start_port + 0x4)
#define RW_LBA_HIGH(channel)         (channel->start_port + 0x5)
#define RW_DEVICE(channel)           (channel->start_port + 0x6)
#define R_STATUS_W_CMD(channel)      (channel->start_port + 0x7)
#define R_ALT_W_CTL(channel)         (channel->start_port + 0x206)

//alternate status寄存器
#define ALT_STATUS_BUSY         0x80
#define ALT_STATUS_DRIVER_READY 0x40
#define ALT_STATUS_DATA_READY   0x08

//device寄存器
#define DEV_MBS 0xa0
#define DEV_LBA 0x40
#define DEV_DEV 0x10

//hard disk operations
#define CMD_IDENTIFY        0xec
#define CMD_READ_SECTOR     0x20
#define CMD_WRITE_SECTOR    0x30
#define BYTES_PER_SECTOR    512

#define MAX_LBA  ((80 * 1024 * 1024 / 512) - 1)


uint8_t channel_cnt;
ide_channel channels[CHANNEL_DEVICE_CNT];

//总扩展分区的lba起始地址
int32_t ext_lba_base = 0;

//记录硬盘主分区和逻辑分区的下标
uint8_t master_no = 0, logical_no = 0;

//分区队列
list partition_list;


//根据传入的disk，通过其对应的channel，给响应IO端口写入数据，选择读写的硬盘
static void select_disk(disk *d) {
    uint8_t device = DEV_MBS | DEV_LBA | (d->no == 0 ? 0 : DEV_DEV);
    outb(RW_DEVICE(d->channel), device);
}

//在disk中，指定待读写的扇区数cnt
static void count_out(disk *d, uint8_t cnt) {
    ide_channel *channel = d->channel;
    outb(RW_SECTOR_CNT(channel), cnt);
}

//在disk中，选择起始地址为lba的扇区
//通过向disk d对应的channel写入对应寄存器的值
static void select_sector(disk *d, uint32_t lba) {
    ASSERT(lba <= MAX_LBA);
    ide_channel *channel = d->channel;
    outb(RW_LBA_LOW(channel), lba & 0x000000ff);
    outb(RW_LBA_MID(channel), (lba & 0x0000ff00) >> 8);
    outb(RW_LBA_HIGH(channel), (lba & 0x00ff0000) >> 16);
    
    //lba的24-27位存储在device寄存器0-3位
    uint8_t device = DEV_MBS | 
                     DEV_LBA | 
                     (d->no == 0 ? 0 : DEV_DEV) | 
                     ((lba & 0x0f000000) >> 24);
    outb(RW_DEVICE(channel), device);
}

//给channel发送命令cmd
static void send_cmd(ide_channel *channel, uint8_t cmd) {
    //写入命令，等待硬盘控制器发来的中断
    channel->waiting_intr = true;
    outb(R_STATUS_W_CMD(channel), cmd);
}

//从disk d中读取数据到buf中，待读取的数据扇区数为cnt
static void disk_read(disk *d, void *buf, uint8_t cnt) {
    uint32_t cnt_ = cnt == 0 ? 256 : cnt;
    uint32_t words = cnt_ * BYTES_PER_SECTOR / 2;
    insw(RW_DATA(d->channel), buf, words);
}

//向disk d写入数据，数据保存在buf中，待写入数据扇区数为cnt
static void disk_write(disk *d, void *buf, uint8_t cnt) {
    uint32_t cnt_ = cnt == 0 ? 256 : cnt;
    uint32_t words = cnt_ * BYTES_PER_SECTOR / 2;
    outsw(RW_DATA(d->channel), buf, words);
}

//等待30s，如果硬盘没有响应，那么返回false。否则，返回硬盘是否正常执行
static bool spin_wait(disk *d) {
    uint32_t ms = 30 * 1000; //max seconds to wait
    while (ms > 0) {
        uint8_t status = inb(R_STATUS_W_CMD(d->channel));
        if (!(status & ALT_STATUS_BUSY)) {
            return !!(status & ALT_STATUS_DATA_READY);
        }
        sleep_by_msecond(10);
        ms -= 10;
    }
    return false;
}

//从硬盘中，以lba为扇区起始地址，读取cnt个扇区的数据到buf中
void ide_read(disk *d, uint32_t lba, void *buf, uint32_t cnt) {
    ide_channel *channel = d->channel;
    mutex_lock(&channel->mutex);

    select_disk(d);

    uint32_t offset = 0;
    while (offset < cnt) {
        //磁盘一次最多读取256个扇区，尽可能读取256
        uint32_t curr_cnt = (offset + 256 < cnt) ? (cnt - offset) : 256;
        count_out(d, curr_cnt);
        select_sector(d, lba + offset);
        send_cmd(channel, CMD_READ_SECTOR);

        //此处需要等待中断信号
        sem_down(&channel->sem);
        if (spin_wait(d) == false) {
            char error[128];
            sprintf(error, "%s read sector %d failed!\n", d->name, lba);
            PANIC(error);
        }

        void *curr_buf = (void*)((uint32_t)buf + offset * BYTES_PER_SECTOR);
        disk_read(d, curr_buf, curr_cnt);
        offset += curr_cnt;
    }

    mutex_unlock(&channel->mutex);
}

//将buf中的数据，写入到以磁盘中以lba为扇区起始地址的cnt个扇区的磁盘空间中
void ide_write(disk *d, uint32_t lba, void *buf, uint32_t cnt) {
    ide_channel *channel = d->channel;
    mutex_lock(&channel->mutex);

    select_disk(d);

    uint32_t offset = 0;
    while (offset < cnt) {
        uint32_t curr_cnt = (offset + 256 < cnt) ? (cnt - offset) : 256;
        count_out(d, curr_cnt);
        select_sector(d, lba + offset);
        send_cmd(channel, CMD_WRITE_SECTOR);

        //检测硬盘是否可用
        if (spin_wait(d) == false) {
            char error[128];
            sprintf(error, "%s read sector %d failed!\n", d->name, lba);
            PANIC(error);
        }

        void *curr_buf = (void*)((uint32_t)buf + offset * BYTES_PER_SECTOR);
        disk_write(d, curr_buf, curr_cnt);
        sem_down(&channel->sem);

        offset += curr_cnt;
    }

    mutex_unlock(&channel->mutex);
}

//将以word为单位的数据转换为字节序为小端序的数据
//用于将identify命令获取的硬盘信息（以字为单位）的处理
static void word_to_bytes(const char *src, char *buf, uint32_t len) {
    for (int i = 0; i < len; i += 2) {
        buf[i+1] = src[i];
        buf[i] = src[i+1];
    }
    buf[len] = '\0';
}

//获取硬盘参数信息
static void print_disk_info(disk *d) {
    char info[BYTES_PER_SECTOR];
    select_disk(d);
    send_cmd(d->channel, CMD_IDENTIFY);
    sem_down(&d->channel->sem);
    
    if (spin_wait(d) == false) {
        char error[128];
        sprintf(error, "%s identify command failed!\n", d->name);
        PANIC(error);
    }

    disk_read(d, info, 1);

    //只需要identify命令返回数据的前64bytes
    char buf[64];
    uint32_t seq_no_start = 10 * 2, seq_no_len = 20;
    word_to_bytes(info + seq_no_start, buf, seq_no_len);
    printk("disk %s information :\n", d->name);
    printk("Sequence Number: %s\n", buf);

    uint32_t type_no_start = 27 * 2, type_no_len = 40;
    memset(buf, 0, 64);
    word_to_bytes(info + type_no_start, buf, type_no_len);
    printk("Type Number: %s\n", buf);

    uint32_t sectors = *(uint32_t*)(info + 60 * 2);
    printk("Sectors: %d\n", sectors);
    printk("Capacity: %dMB\n", sectors * BYTES_PER_SECTOR / 1024 / 1024);
}

//扫描磁盘中地址为lba的扇区中的所有分区
static void scan_partition(disk *d, uint32_t lba) {

}

//打印磁盘分区信息
static bool print_partition_info(list_node *node, int arg) {

}

//磁盘中断处理程序
void disk_intr_handler(uint8_t irq) {
    uint8_t no = irq - 0x2e;
    ASSERT(no == 0 || no == 1);

    ide_channel *channel = &channels[no];
    if (channel->waiting_intr) {
        sem_up(&channel->sem);
        channel->waiting_intr = false;

        //通知磁盘控制器，以及完成当前工作，可以进行其他任务
        inb(R_STATUS_W_CMD(channel)); 
    }
}

void ide_channel_init() {
    uint8_t disk_cnt = *((uint8_t*)DISK_CNT_POINTER);
    channel_cnt = DIV_ROUND_UP(disk_cnt, 2);

    printk("ide start\n");
    ide_channel *channel;
    for (int i = 0; i < channel_cnt; ++i) {
        channel = &channels[i];

        if (i == 0) {
            //channel0，对应从片IRQ14
            channel->start_port = 0x1f0;
            channel->irq_no = 0x20 + 14;
        }
        if (i == 1) {
            //channel1，对应从片IRQ15
            channel->start_port = 0x170;
            channel->irq_no = 0x20 + 15;
        }

        channel->waiting_intr = false;
        mutex_init(&channel->mutex);
        sem_init(&channel->sem, 0);
        intr_set_handler(channel->irq_no, disk_intr_handler);

        for (int j = 0; j < 2; ++j) {
            disk *d = &channel->devices[j];
            d->channel = channel;
            d->no = j;
            sprintf(d->name, "sd%c", 'a' + i * 2 + j);

            print_disk_info(d);

            //slave disk
            if (d->no != 0) {
                scan_partition(d, 0);
            }
            master_no = 0;
            logical_no = 0;
        }
    }
}