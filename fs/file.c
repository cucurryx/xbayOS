#include <file.h>
#include <printk.h>
#include <disk.h>
#include <fs.h>
#include <dir.h>
#include <debug.h>

extern partition *curr_part;

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

//创建文件，文件名为filename，位于目录parent下，文件标识flag
//如果创建成功，返回文件描述符，否则返回-1
int32_t create_file(dir_t *parent, char *filename, uint8_t flag) {
    uint8_t *buf = (uint8_t*)sys_malloc(SECTOR_SIZE * 2);
    ASSERT(buf != NULL);

    //分配inode_no和inode，并初始化
    int32_t inode_no = inode_bm_alloc(curr_part);
    ASSERT(inode_no != -1);

    inode_t *new_inode = (inode_t*)sys_malloc(sizeof(inode_t));
    ASSERT(new_inode != NULL);
    inode_init(inode_no, new_inode);

    //注册文件表
    int fidx = file_table_alloc();
    file_table[fidx].fd_inode = new_inode;
    file_table[fidx].fd_offset = 0;
    file_table[fidx].fd_flag = flag;
    file_table[fidx].fd_inode->writing = false;

    dir_entry entry;
    memset(&entry, 0, sizeof(dir_entry));
    set_entry(filename, inode_no, ET_FILE, &entry);

    ASSERT(add_entry(parent, &entry, buf));

    //更新bitmap, inode等到硬盘
    memset(buf, 0, sizeof(buf));
    inode_sync(curr_part, parent->inode, buf);

    memset(buf, 0, sizeof(buf));
    inode_sync(curr_part, new_inode, buf);

    bitmap_sync(curr_part, inode_no, INODE_BITMAP);

    list_push_back(&curr_part->open_inodes, &new_inode->i_node);

    new_inode->open_cnt = 1;

    sys_free(buf);

    int32_t fd = fd_install(fidx);

    return fd;
}
