#include <disk.h>
#include <global.h>
#include <io.h>
#include <debug.h>
#include <timer.h>
#include <stdio.h>
#include <printk.h>
#include <interrupt.h>
#include <memory.h>

//读其磁盘30s，如果超时没有正常响应，则报错
#define SPIN_WAIT_ERROR(format, args)               \
    do {                                            \
        if (spin_wait(d) == false) {                \
            char error[128];                        \
            sprintf(error, format, args);           \
            PANIC(error);                           \
        }                                           \
    } while (0);


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
#define ALT_STATUS_DATA_READY        0x08

//device寄存器
#define DEV_MBS 0xa0
#define DEV_LBA 0x40
#define DEV_DEV 0x10

//hard disk operations

//identify command
#define CMD_IDENTIFY        0xec

//read sector command
#define CMD_READ_SECTOR     0x20

//write sector command
#define CMD_WRITE_SECTOR    0x30

//obviously
#define BYTES_PER_SECTOR    512

//LBA最大值
#define MAX_LBA  ((80 * 1024 * 1024 / 512) - 1)

//引导扇区中代码可用空间长度
#define BOOT_SECTOR_CODE_LEN 446

#define DEFAULT_DISK_NUM 4

typedef struct __pt_entry pt_entry;
typedef struct __boot_sector boot_sector;

//partition table entry
struct __pt_entry {
    //是否可以boot
    uint8_t flag;

    //起始磁头号
    uint8_t start_head;

    //起始扇区号
    uint8_t start_sec;

    //起始柱面号
    uint8_t start_chs;

    //分区类型
    uint8_t fs_type;

    //结束磁头号
    uint8_t end_head;

    //结束扇区号
    uint8_t end_sec;

    //结束柱面号
    uint8_t end_chs;

    //分区起始扇区的LBA地址
    uint32_t start_lba;

    //分区的扇区数
    uint32_t sec_cnt;
} __attribute__ ((packed));

//引导扇区struct
struct __boot_sector {
    uint8_t code[BOOT_SECTOR_CODE_LEN];
    pt_entry pt[DEFAULT_DISK_NUM];
    uint16_t magic_num;
} __attribute__ ((packed));

//通道数
uint8_t channel_cnt;

//通道数组，两个通道，从片IRQ14和IRQ15
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
        uint32_t curr_cnt = (offset + 256 < cnt) ? 256 : (cnt - offset);

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

    //等待硬盘中断处理函数执行
    sem_down(&d->channel->sem);

    SPIN_WAIT_ERROR("%s identify command failed!\n", d->name);

    disk_read(d, info, 1);

    //只需要identify命令返回数据的前64bytes
    char buf[64];
    uint32_t seq_no_start = 10 * 2, seq_no_len = 20;
    word_to_bytes(info + seq_no_start, buf, seq_no_len);

    printk("\ndisk %s information :", d->name);
    printk("\nSequence Number: %s", buf);

    uint32_t type_no_start = 27 * 2, type_no_len = 40;
    memset(buf, 0, 64);
    word_to_bytes(info + type_no_start, buf, type_no_len);

    printk("\nType Number: %s", buf);

    uint32_t sectors = *(uint32_t*)(info + 60 * 2);

    printk("\nSectors: %d", sectors);
    printk("\nCapacity: %dMB\n\n\n", sectors * BYTES_PER_SECTOR / 1024 / 1024);
}


//扫描磁盘中地址为lba的扇区中的所有分区
static void scan_partition(disk *d, uint32_t lba) {
    boot_sector *boot = sys_malloc(sizeof(boot_sector));

    ide_read(d, lba, (void*)boot, 1);

    pt_entry *pt = boot->pt;

    for (int i = 0; i < DEFAULT_DISK_NUM; ++i) {

        //扩展分区
        if (pt->fs_type == 0x5) {
            if (ext_lba_base != 0) {
                scan_partition(d, pt->start_lba + ext_lba_base);
            } else {
                ext_lba_base = pt->start_lba;
                scan_partition(d, pt->start_lba);
            }

        } else if (pt->fs_type != 0) {

            partition *part;

            //主分区
            if (lba == 0) {
                part = &d->prim_parts[master_no];

                part->start = lba + pt->start_lba;
                part->count = pt->sec_cnt;
                part->belong_disk = d;
                sprintf(part->name, "%s%d", d->name, master_no + 1);

                // ASSERT(list_exist(&partition_list, &part->part_tag) == false);
                // list_push_back(&partition_list, &part->part_tag);

                ASSERT(master_no < 3);
                ++master_no;

            //逻辑分区
            } else {
                part = &d->logic_parts[logical_no];

                part->start = lba + pt->start_lba;
                part->count = pt->sec_cnt;
                part->belong_disk = d;
                sprintf(part->name, "%s%d", d->name, logical_no + 5);

                ++logical_no;
                if (logical_no >= 8) {
                    return;
                }

            }

            //打印该分区信息
            printk("\n%s start lba: 0x%x, sector count: 0x%x", 
                            part->name, 
                            part->start, 
                            part->count);
        }

        ++pt;
    }

    sys_free(boot);
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

    ide_channel *channel;
    for (int i = 0; i < channel_cnt; ++i) {
        channel = &channels[i];

        if (i == 0) {
            //channel0，对应从片IRQ14
            channel->start_port = 0x1f0;
            channel->irq_no = 0x20 + 14;
        }

#ifdef TWO_DISKS

        if (i == 1) {
            //channel1，对应从片IRQ15
            channel->start_port = 0x170;
            channel->irq_no = 0x20 + 15;
        }

#endif // TWO_DISKS

        channel->waiting_intr = false;
        mutex_init(&channel->mutex);
        sem_init(&channel->sem, 0);

        //设置硬盘中断处理函数
        intr_set_handler(channel->irq_no, disk_intr_handler);

        for (int j = 0; j < 2; ++j) {
            disk *d = &channel->devices[j];
            d->channel = channel;
            d->no = j;

            memset(d->name, 0, NAME_LEN);
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