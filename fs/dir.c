#include <dir.h>
#include <memory.h>
#include <debug.h>
#include <file.h>
#include <printk.h>


//根目录
dir_t root;

//挂载了的当前分区
extern partition *curr_part;


//打开根目录
void open_root(partition *part) {
    root.inode = inode_open(part, part->sb->root_inode_no);
    root.offset = 0;
}

//打开目录，需要打开对应inode_no对应的inode
dir_t *open_dir(partition *part, uint16_t inode_no) {
    dir_t *dir = (dir_t*)sys_malloc(sizeof(dir_t));
    dir->inode = inode_open(part, inode_no);
    dir->offset = 0;
    return dir;
}

//关闭目录，需要关闭dir对应的inode，并释放内存
//不关闭根目录
void close_dir(dir_t *dir) {
    if (dir == &root) {
        return;
    } else {
        inode_close(dir->inode);
        sys_free(dir);
    }
}   

//设置目录项
void set_entry(char *name, uint32_t inode_no, 
                      entry_type type, dir_entry *e) {
    uint8_t name_len = strlen(name);
    ASSERT(name_len <= FILENAME_LEN);

    memcpy(e->name, name, name_len);
    e->inode_no = inode_no;
    e->type = type;
}

//同步目录项，将目录项e写入目录parent中
bool add_entry(dir_t *parent, dir_entry *e, void *buf) {
    disk *d = curr_part->belong_disk;
    inode_t *inode = parent->inode;
    uint32_t *isec = inode->i_sectors;

    uint32_t dir_size = inode->i_size;
    uint32_t entry_size = sizeof(dir_entry);
    uint32_t blk_cnt = DIRECT_BLK_CNT + SECTOR_SIZE / BLK_POINTER_SIZE;
    uint32_t blk_bm_idx = -1;
    uint32_t start_lba = curr_part->sb->start_lba;
    uint32_t data_start = curr_part->sb->data_start;

    uint32_t *all_blks = (uint32_t*)sys_malloc(blk_cnt * BLK_POINTER_SIZE);
    memset(all_blks, 0, sizeof(all_blks));

    for (int i = 0; i < blk_cnt; ++i) {
        blk_bm_idx = -1;

        //空闲块
        if (all_blks[i] == 0) {
            uint32_t lba = block_bm_alloc(curr_part);
            ASSERT(lba != -1);

            blk_bm_idx = lba - start_lba;
            ASSERT(blk_bm_idx != -1);

            bitmap_sync(curr_part, blk_bm_idx, BLOCK_BITMAP);

            //直接块，直接分配，并同步
            if (i < 12) {
                isec[i] = lba;
                all_blks[i] = lba;

            } else {

                //一级间接块未分配
                if (i == DIRECT_BLK_CNT) {

                    isec[12] = lba;
                    lba = block_bm_alloc(curr_part);

                    //分配失败
                    if (lba == -1) {
                        blk_bm_idx = isec[12] - start_lba;
                        bitmap_clear(&curr_part->block_bm, blk_bm_idx);
                        isec[12] = 0;
                        PANIC("allocate block bitmap failed!\n");
                    }
                    
                    blk_bm_idx = lba - data_start;
                    ASSERT(blk_bm_idx != -1);

                    bitmap_sync(curr_part, blk_bm_idx, BLOCK_BITMAP);
                }

                all_blks[12] = lba;
                ide_write(d, isec[12], all_blks + 12, 1);
            }

            memset(buf, 0, SECTOR_SIZE);
            memcpy(buf, e, sizeof(dir_entry));
            ide_write(d, all_blks[i], buf, 1);
            inode->i_size += sizeof(dir_entry);

            sys_free(all_blks);
            return true;
        }

        ide_read(d, all_blks[i], buf, 1);
        
        for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry); ++j) {
            dir_entry *entry = (dir_entry*)buf;

            if (entry[j].type == ET_UNKOWN) {
                
                memcpy(&entry[j], e, sizeof(dir_entry));
                ide_write(d, all_blks[i], buf, 1);

                inode->i_size += sizeof(dir_entry);

                sys_free(all_blks);
                return true;
            }
        }
    }
#ifndef NODEBUG
    printk("directory is full\n");
#endif // NODEBUG

    sys_free(all_blks);
    return false;
}

//读取分区part下目录dir对应的inode的所有块
//保存在堆分配的内存中，返回指针
static uint32_t *read_blocks(partition *part, dir_t *dir) {
    uint32_t direct = DIRECT_BLK_CNT;
    uint32_t blk_cnt = direct + SECTOR_SIZE / BLK_POINTER_SIZE;

    uint32_t *all_blks = (uint32_t*)sys_malloc(blk_cnt * BLK_POINTER_SIZE);
    memset(all_blks, 0, sizeof(all_blks));

    inode_t *inode = dir->inode;
    uint32_t *isec = inode->i_sectors;
    disk *d = part->belong_disk;

    ASSERT(all_blks != NULL);

    for (int i = 0; i < direct; ++i) {
        all_blks[i] = isec[i];
    }

    //存在一级间接块表
    if (isec[direct] != 0) {
        ide_read(d, isec[direct], all_blks + direct, 1);
    }

    return all_blks;
}

//遍历目录项表table，找到名字为name的目录项，并拷贝到e中返回，并返回true
//如果不存在，那么e为NULL，并返回false
static bool find_entry(dir_entry *table, const char *name, dir_entry *e) {
    uint32_t entry_cnt = SECTOR_SIZE / sizeof(dir_entry);

    //遍历该块中所有dir entry
    for (int j = 0; j < entry_cnt; ++j) {
        dir_entry *entry = &table[j];

        //找到name目录项
        if (strcmp(name, entry->name) == 0) {
            memcpy(e, entry, sizeof(dir_entry));
            return true;
        }
    }   

    e = NULL;
    return false;
}

//判断dir下是否存在name这个文件或者目录
//如果是，返回true，并将其对应的dir_entry保存在e中返回
//否则，返回false，e设置为NULL
bool dir_exist(partition *part, dir_t *dir, 
                const char *name, dir_entry *e) {

    disk *d = part->belong_disk;
    uint32_t blk_cnt = DIRECT_BLK_CNT + SECTOR_SIZE / BLK_POINTER_SIZE;
    uint32_t *all_blks = read_blocks(part, dir);
    uint8_t *buf = (uint8_t*)sys_malloc(SECTOR_SIZE);

    //遍历所有块
    for (int i = 0; i < blk_cnt; ++i) {
        if (all_blks[i] == 0) {
            continue;
        }

        ide_read(d, all_blks[i], buf, 1);

        if (find_entry((dir_entry*)buf, name, e)) {
            break;
        }

        memset(buf, 0, SECTOR_SIZE);
    }

    sys_free(buf);
    sys_free(all_blks);

    return !(e == NULL);
}
