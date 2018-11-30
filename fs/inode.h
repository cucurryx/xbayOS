#ifndef __FS_INODE_H
#define __FS_INODE_H

#include <stdint.h>
#include <list.h>
#include <disk.h>

//每个inode中对应的sector数组大小
//其中12个直接块，另外一个一级间接块索引
#define INODE_SECTOR_CNT 13


typedef struct __inode_t inode_t;

//inode
struct __inode_t {
    //用于加入inode list
    list_node i_node;
    
    //inode编码
    uint32_t i_no;

    //文件大小或者目录项数
    uint32_t i_size;

    //打开次数
    uint32_t open_cnt;

    //是否在写文件，避免同时写
    bool writing;

    //直接块和一级间接指针
    uint32_t i_sectors[INODE_SECTOR_CNT];
};

void inode_sync(partition *part, inode_t *inode, void *buf);
inode_t *inode_open(partition *part, uint32_t no);
void inode_close(inode_t *inode);
void inode_init(uint32_t no, inode_t *inode);

#endif // !__FS_INODE_H
