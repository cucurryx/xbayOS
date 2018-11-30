#include <file.h>
#include <printk.h>
#include <disk.h>
#include <fs.h>


//文件表
file file_table[OPEN_FILE_MAX];

//从文件表file_table中分配一个空间位置，返回其下标
//如果失败，返回-1
int32_t file_table_alloc() {
    for (int i = USER_FILE_TABLE_START; i < OPEN_FILE_MAX; ++i) {
        if (file_table[i].fd_inode == NULL) {
            return i;
        }
    }
    printk("can't open more file!\n");
    return -1;
}

//将文件表表项对应的文件表下标ft_idx，安装到当前进程的fd_table中，返回其下标
//如果失败，返回-1
int32_t fd_install(int32_t ft_idx) {
    task_struct *curr = running_thread();

    for (int i = USER_FILE_TABLE_START; i < FD_MAX; ++i) {
        if (curr->fd_table[i] == -1) {
            curr->fd_table[i] = ft_idx;
            return i;
        } 
    }

    printk("no more file descriptor for this process!\n");
    return -1;
}

//分配一个inode，返回其inode序号
int32_t inode_bm_alloc(partition *part) {
    int32_t idx = bitmap_scan(&part->inode_bm, 1);
    if (idx != -1) {
        bitmap_set(&part->inode_bm, idx);
        return idx;
    } else {
        return -1;
    }
}

//分配一个block，返回其地址
int32_t block_bm_alloc(partition *part) {
    int32_t idx = bitmap_scan(&part->block_bm, 1);
    if (idx == -1) {
        bitmap_set(&part->block_bm, idx);
        return part->sb->start_lba + idx;
    } else {
        return -1;
    }
}

//同步位图到硬盘，参数分别为分区part、位索引idx和位图类型type
//将idx位所在的512字节同步到硬盘
void bitmap_sync(partition *part, uint32_t idx, uint8_t type) {
    uint32_t sector = idx / BITS_PER_SECTOR;
    uint32_t bytes = idx / 8;
    uint8_t *offset;

    if (type == INODE_BITMAP) {
        offset = part->inode_bm.bits + bytes;
        sector += part->sb->in_bmap_addr;
    } else if (type == BLOCK_BITMAP) {
        offset = part->block_bm.bits + bytes;
        sector += part->sb->fb_bmap_addr;
    }

    ide_write(part->belong_disk, sector, offset, 1);
}