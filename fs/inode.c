#include <inode.h>
#include <debug.h>
#include <fs.h>
#include <interrupt.h>

//inode position struct
typedef struct __inode_pos {
    //该inode是否横跨扇区
    bool cross_sector;

    //inode所在扇区LBA
    uint32_t lba;

    //inode所在扇区offset
    uint32_t offset;
} inode_pos;


//获取分区part内，inode no为no的inode所在位置
//返回值放在pos所指向的内存中
static void get_position(partition *part, uint32_t no, inode_pos *pos) {
    ASSERT(no < MAX_FILES_PER_PART);

    uint32_t inode_table = part->sb->in_array_addr;
    uint32_t bytes = no * sizeof(inode_t);

    pos->lba = inode_table + bytes / SECTOR_SIZE;
    pos->offset = bytes % SECTOR_SIZE;

    //cross two sector
    if (pos->offset + sizeof(inode_t) > SECTOR_SIZE) {
        pos->cross_sector = true;
    } else {
        pos->cross_sector = false;
    }
}

//硬盘中不保存状态信息，为避免下次加载混乱，清除inode状态信息
static void clear_status(inode_t *inode) {
    inode->open_cnt = 0;
    inode->writing = false;
    inode->i_node.next = NULL;
    inode->i_node.prev = NULL;
}

//同步inode，将buf中所存inode，从内存同步到硬盘中
void inode_sync(partition *part, inode_t *inode, void *buf) {
    disk *d = part->belong_disk;
    inode_pos pos;
    get_position(part, inode->i_no, &pos);

    inode_t temp;
    memcpy(&temp, inode, sizeof(inode_t));
    clear_status(&temp);

    uint8_t sector_cnt = pos.cross_sector ? 2 : 1;

    ide_read(d, pos.lba, buf, sector_cnt);
    memcpy(buf + pos.offset, &temp, sizeof(inode_t));
    ide_write(d, pos.lba, buf, sector_cnt);
}

//将list_node指针转化为对应的inode_t指针
static inode_t *node_to_inode(list_node *node) {
    return (inode_t*)node;
}

static inode_t *find_in_list(uint32_t no, list *l) {
    list_node *node = l->head.next;
    while (node != &l->tail) {
        if (node_to_inode(node)->i_no == no) {
            return node_to_inode(node);
        }
        node = node->next;
    }
    return NULL;
}

//打开inode，根据inode编号返回对应inode
//如果已经存在于内存，在inode open队列中，直接返回
//否则，从磁盘中读入inode到内存，然后加入到open队列并返回
//为了所有进程共享inode，所以采用kmalloc，从内核空间分配内存
inode_t *inode_open(partition *part, uint32_t no) {
    inode_t *res = find_in_list(no, &part->open_inodes);

    if (res != NULL) {
        ++res->open_cnt;
        return res;
    }

    inode_pos pos;
    get_position(part, no, &pos);

    inode_t *inode = (inode_t*)sys_kmalloc(sizeof(inode_t));
    uint8_t sector = pos.cross_sector ? 2 : 1;
    uint8_t *buf = (uint8_t*)sys_kmalloc(sector * SECTOR_SIZE);
    disk *d = part->belong_disk;
    
    ide_read(d, pos.lba, buf, sector);
    memcpy(inode, buf + pos.offset, sizeof(inode_t));

    //加入到inode open队列
    list_push_front(&part->open_inodes, &inode->i_node);
    inode->open_cnt = 1;

    sys_free(buf);

    return inode;
}

//关闭inode，根据传入inode，从打开inode队列中删除，释放内存
void inode_close(inode_t *inode) {
    intr_stat status;
    INTERRUPT_DISABLE(status);

    --inode->open_cnt;

    //如果引用计数归0，释放内存
    if (inode->open_cnt == 0) {
        list_remove(&inode->i_node);
        sys_kfree(inode);
    }

    INTERRUPT_RESTORE(status);
}

//初始化inode_t结构体
void inode_init(uint32_t no, inode_t *inode) {
    inode->i_no = no;
    inode->i_size = 0;
    inode->open_cnt = 0;
    inode->writing = false;
    
    for (int i = 0; i < INODE_SECTOR_CNT; ++i) {
        inode->i_sectors[i] = 0;
    }
}