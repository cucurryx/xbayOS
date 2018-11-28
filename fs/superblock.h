#ifndef __FS_SUPERBLOCK_H
#define __FS_SUPERBLOCK_H

#include <stdint.h>

//super block所在sector非可用区域size
#define SECTOR_PLACEHOLDER_SIZE 460


typedef struct __super_block super_block;

//超级块
struct __super_block {
    //魔数，标识是否建立了super block
    uint32_t magic_num;

    //数据块数量
    uint32_t block_cnt;

    //inode数量
    uint32_t inode_cnt;

    //分区的起始lba地址
    uint32_t start_lba;

    //空闲块位图地址
    uint32_t fb_bmap_addr;

    //空闲块位图大小
    uint32_t fb_bmap_size;

    //inode位图地址
    uint32_t in_bmap_addr;

    //inode位图大小
    uint32_t in_bmap_size;

    //inode数组地址
    uint32_t in_array_addr;

    //inode数组大小
    uint32_t in_arrat_size;

    //根目录地址
    uint32_t root_inode_no;

    //根目录大小
    uint32_t root_dir_size;

    //为了凑够512byte
    uint8_t placeholder[SECTOR_PLACEHOLDER_SIZE];

} __attribute__ ((packed)) ; 

#endif // !__FS_SUPERBLOCK_H