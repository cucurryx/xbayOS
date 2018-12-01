#include <fs.h>
#include <disk.h>
#include <memory.h>
#include <printk.h>
#include <global.h>
#include <dir.h>
#include <debug.h>

extern ide_channel channels[CHANNEL_DEVICE_CNT];
extern dir_t root;

//默认操作的分区
partition *curr_part;


//初始化supbe block
static void init_super_block(disk *d, partition *part, super_block *sb) {
    uint32_t total_secs = part->count;

    uint32_t inode_bmap_secs = DIV_ROUND_UP(
        MAX_FILES_PER_PART, 
        BITS_PER_SECTOR);
    uint32_t inode_array_secs = DIV_ROUND_UP(
        sizeof(inode_t) * MAX_FILES_PER_PART, 
        BITS_PER_SECTOR);

    uint32_t fb_secs = total_secs - 2 - inode_bmap_secs - inode_array_secs;

    uint32_t fb_bmap_secs = DIV_ROUND_UP(fb_secs, BITS_PER_SECTOR);
    fb_bmap_secs = DIV_ROUND_UP(fb_secs - fb_bmap_secs, BITS_PER_SECTOR);

    sb->magic_num = 0xcafebebe;

    sb->block_cnt = total_secs;

    sb->inode_cnt = MAX_FILES_PER_PART;

    sb->start_lba = part->start;

    sb->fb_bmap_addr = sb->start_lba + 2;
    sb->fb_bmap_size = fb_bmap_secs;
    
    sb->in_bmap_addr = sb->fb_bmap_addr + sb->fb_bmap_size;
    sb->in_bmap_size = inode_bmap_secs;
    
    sb->in_array_addr = sb->in_bmap_addr + sb->in_bmap_size;
    sb->in_array_size = inode_array_secs;

    sb->data_start = sb->in_array_addr + sb->in_array_addr;
    sb->root_inode_no = 0;
    sb->root_dir_size = sizeof(dir_entry);

    printk("\n%s superblock build ok", part->name);
}
 
//初始化根目录，root dir位于数据块的第一个块，inode序号为0
static void set_root_dir(void *buf, 
                         uint32_t buf_size, 
                         disk *d, 
                         super_block *sb) {
    //root directory
    memset(buf, 0, buf_size);
    dir_entry *root = (dir_entry*)buf;

    memset(root->name, '.', 1);
    root->inode_no = 0;
    root->type = ET_DIRECTORY;
    ++root;

    memset(root->name, '.', 2);
    root->inode_no = 0;
    root->type = ET_DIRECTORY;
    
    ide_write(d, sb->data_start, buf, 1);
}

//初始化inode数组
static void set_inode_array(void *buf, 
                            uint32_t buf_size, 
                            disk *d, 
                            super_block *sb) {
    //inode array
    memset(buf, 0, buf_size);
    inode_t *i = (inode_t*)buf;
    i->i_size = sb->root_dir_size * 2;
    i->i_no = 0;
    i->i_sectors[0] = sb->data_start;
    ide_write(d, sb->in_array_addr, buf, sb->in_array_size);
}

static void set_inode_bitmap(void *buf,
                             uint32_t buf_size,
                             disk *d,
                             super_block *sb) {
    //inode bitmap
    memset(buf, 0, buf_size);
    ((uint8_t*)buf)[0] |= 0x1;
    ide_write(d, sb->in_bmap_addr, buf, sb->in_array_size);
}

static void set_free_block_bitmap(uint8_t *buf,
                                  uint32_t buf_size,
                                  disk *d,
                                  super_block *sb,
                                  partition *part) {
    uint32_t total_secs = part->count;
    uint32_t inode_bmap_secs = DIV_ROUND_UP(
        MAX_FILES_PER_PART, 
        BITS_PER_SECTOR);
    uint32_t inode_array_secs = DIV_ROUND_UP(
        sizeof(inode_t) * MAX_FILES_PER_PART, 
        BITS_PER_SECTOR);
    uint32_t fb_secs = total_secs - 2 - inode_bmap_secs - inode_array_secs;
    uint32_t fb_bmap_secs = DIV_ROUND_UP(fb_secs, BITS_PER_SECTOR);
    uint32_t fb_bmap_len = fb_secs - fb_bmap_secs;

    //block bitmap 第0块分配给根目录
    buf[0] |= 0x1;

    uint32_t last_byte = fb_bmap_len / 8;
    uint8_t last_bit = fb_bmap_len % 8;
    uint32_t last_size = SECTOR_SIZE - (last_byte % SECTOR_SIZE);

    //设置位图的最后字节
    memset(&buf[last_byte], 0xff, last_size);
    for (int i = 0; i <= last_bit; ++i) {
        buf[last_byte] &= ~(1 << i);
    }

    ide_write(d, sb->fb_bmap_addr, buf, sb->fb_bmap_size);
}

//格式化分区，创建文件系统
// ----------------------------------------------------------------
// | boot  | super | free block | inode  | inode | root |  free   |
// | block | block |   bitmap   | bitmap | array |  dir |  blocks |
// ----------------------------------------------------------------
static void build_fs(disk *d, partition *part) {
    super_block sb;
    init_super_block(d, part, &sb);

    //将初始化了的superblock写入disk
    ide_write(d, part->start + 1, &sb, 1);

    uint32_t buf_size = max(sb.fb_bmap_size, sb.in_bmap_size);
    uint8_t *buf = (uint8_t*)sys_malloc(buf_size);

    set_free_block_bitmap(buf, buf_size, d, &sb, part);
    
    set_inode_bitmap(buf, buf_size, d, &sb);
    
    set_inode_array(buf, buf_size, d, &sb);
    
    set_root_dir(buf, buf_size, d, &sb);

    sys_free(buf);
}

//挂载默认分区，将该分区文件系统元信息读入内存
//分别读入superblock, block bitmap, inode bitmap
// ----------------------------------------------------------------
// | boot  | super | free block | inode  | inode | root |  free   |
// | block | block |   bitmap   | bitmap | array |  dir |  blocks |
// ----------------------------------------------------------------
void mount_part(partition *part) {
    curr_part = part;
    disk *d = curr_part->belong_disk;


    //磁盘读写必须以sector为单位
    super_block *buf = (super_block*)sys_malloc(SECTOR_SIZE);
    ASSERT(buf != NULL);

    memset(buf, 0, SECTOR_SIZE);
    ide_read(d, curr_part->start + 1, buf, 1);


    curr_part->sb = (super_block*)sys_malloc(sizeof(super_block));
    memset(curr_part->sb, 0, sizeof(super_block));
    memcpy(curr_part->sb, buf, sizeof(super_block));

    //设置block bitmap
    uint32_t bytes = buf->fb_bmap_size * SECTOR_SIZE;
    curr_part->block_bm.bits = (uint8_t*)sys_malloc(bytes);
    curr_part->block_bm.length = bytes;
    ASSERT(curr_part->block_bm.bits != NULL);

    ide_read(d, buf->fb_bmap_addr, curr_part->block_bm.bits, buf->fb_bmap_size);

    //设置inode bitmap
    bytes = buf->in_bmap_size * SECTOR_SIZE;
    curr_part->inode_bm.bits = (uint8_t*)sys_malloc(bytes);
    curr_part->inode_bm.length = bytes;
    ASSERT(curr_part->inode_bm.bits != NULL);

    ide_read(d, buf->in_bmap_addr, curr_part->inode_bm.bits, buf->in_bmap_size);

    list_init(&curr_part->open_inodes);

#ifndef NODEBUG
    printk("%s mount ok! the block bitmap size: %d, "
           "the inode bitmap size: %d\n", 
           curr_part->name,
           curr_part->block_bm.length, 
           curr_part->inode_bm.length);
#endif // !NODEBUG

}

//文件系统初始化
void fs_init() {
    super_block *sb = (super_block*)sys_malloc(BLOCK_SIZE);

    ide_channel *channel = &channels[0];
    
    //hd80m.img
    disk *d = &channel->devices[1];

    char *default_part_name = "sdb1";

    partition *part = d->prim_parts;
    for (int i = 0; i < PRIM_PARTS_CNT; ++i) {
        if (part->count != 0) {
            memset(sb, 0, SECTOR_SIZE);
            ide_read(d, part->start + 1, sb, 1);

            //如果存在文件系统，就不需要build_fs
            if (sb->magic_num == 0xcafebebe) {
#ifndef NODEBUG
                printk("%s file system ok!\n", part->name);
#endif // !NODEBUG

            } else {
                build_fs(d, part);
#ifndef NODEBUG
                printk("%s build file system.\n", part->name);
#endif // !NODEBUG

            }

            //挂载name为default_part_name的分区
            if (strcmp(part->name, default_part_name) == 0) {
                mount_part(part);
            }

        }
        ++part;
    }
    
    part = d->logic_parts;

    for (int i = 0; i < LOGIC_PARTS_CNT; ++i) {
        if (part->count != 0) {
            memset(sb, 0, SECTOR_SIZE);
            ide_read(d, part->start + 1, sb, 1);

            //如果存在文件系统，就不需要build_fs
            if (sb->magic_num == 0xcafebebe) {
#ifndef NODEBUG
                printk("%s file system ok!\n", part->name);
#endif // !NODEBUG

            } else {
                build_fs(d, part);

#ifndef NODEBUG
                printk("%s build file system.\n", part->name);
#endif // !NODEBUG

            }
        }
        ++part;
    }

    sys_free(sb);
}

//解析pathname，并将第一个路径名放在first中返回
static char *parse_path(char *path, char *first) {
    if (path == NULL || *path == '\0') {
        return NULL;
    }

    while (*path == '/') {
        ++path;
    }

    while (*path != '/' && *path != '\0') {
        *first = *path;
        ++path;
        ++first;
    }

    return path;
}

//解析路径深度
uint32_t path_depth(char *path) {
    ASSERT(path != NULL);
    
    uint32_t depth = 0;
    char buf[FILENAME_LEN];

    while (path != NULL && *path != '\0') {
        path = parse_path(path, buf);

        if (*path != NULL && *path != '\0') {
            ++depth;
        } else {
            break;
        }

        memset(buf, 0, FILENAME_LEN);
    }

    return depth;
}

//判断路径是否为根目录
//如果是，设置record并返回true
static bool is_root(const char *path, path_record *record) {
    char *root_paths[3] = {
        "/",
        "/..",
        "/."
    };

    for (int i = 0; i < 3; ++i) {
        if (strcmp(path, root_paths[i]) == 0) {
            record->parent = &root;
            memset(record->prev, 0, sizeof(record->prev));
            record->type = FT_DIRECTORY;
            return true;
        }
    }
    return false;
}

static bool path_valid(const char *path) {
    uint32_t path_len = strlen(path);
    return path_len > 1 && path[0] == '/' && path_len < PATH_MAXLEN;
}

//搜索文件path，如果成功，返回其inode号，否则返回-1
//其中搜索过的路径记录在record中
static uint32_t search_file(const char *path, path_record *record) {

    if (is_root(path, record)) {
        return ROOT_INODE; 
    }

    ASSERT(path_valid(path));

    char *sub = (char*)path;
    dir_t *parent = &root;
    dir_entry entry;

    char name[FILENAME_LEN] = {0};

    record->parent = parent;
    record->type = FT_UNKOWN;
    uint32_t pinode_no = 0;

    sub = parse_path(sub, name);

    while (name[0] != '\0') {
        
        //记录查询过的路径
        strcat(record->prev, "/");
        strcat(record->prev, name);

        //parent的目录项中存在name
        if (dir_exist(curr_part, parent, name, &entry)) {

            memset(name, 0, FILENAME_LEN);
            if (sub) {
                sub = parse_path(sub, name);
            }

            //parse返回NULL
            switch(entry.type) {
                //目录，继续
                case ET_DIRECTORY:
                    pinode_no = parent->inode->i_no;
                    close_dir(parent);
                    parent = open_dir(curr_part, entry.inode_no);
                    record->parent = parent;
                    break;
                //普通文件
                case ET_FILE:
                    record->type = FT_FILE;
                    return entry.inode_no;
                default:
                    break;
            }
        } else {
            return -1;
        }
    }

    close_dir(record->parent);

    //更新record
    record->parent = open_dir(curr_part, pinode_no);
    record->type = FT_DIRECTORY;

    return entry.inode_no;
}