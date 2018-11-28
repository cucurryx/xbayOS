#ifndef __FS_FS_H
#define __FS_FS_H

#include <stdint.h>
#include <list.h>
#include <superblock.h>



//每个inode中对应的sector数组大小
//其中12个直接块，另外一个一级间接块索引
#define INODE_SECTOR_CNT 13

//每个扇区的bits数
//512Byte = 4096bit
#define BITS_PER_SECTOR 4096

//每个分区的最大inode数
#define MAX_FILES_PER_PART 4096

//扇区大小
#define SECTOR_SIZE 512

//块大小，一般块为n个分区，此处n == 1
#define BLOCK_SIZE 512


typedef struct __inode_t inode_t;


//inode
struct __inode_t {
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
    
    //用于加入inode list
    list_node i_node;
};


void fs_init();


#endif // !__FS_FS_H