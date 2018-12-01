#ifndef __FS_FS_H
#define __FS_FS_H

#include <stdint.h>
#include <list.h>
#include <superblock.h>
#include <inode.h>


//每个扇区的bits数
//512Byte = 4096bit
#define BITS_PER_SECTOR 4096

//每个分区的最大inode数
#define MAX_FILES_PER_PART 4096

//扇区大小
#define SECTOR_SIZE 512

//块大小，一般块为n个分区，此处n == 1
#define BLOCK_SIZE 512


void fs_init();

uint32_t path_depth(char *path);

#endif // !__FS_FS_H