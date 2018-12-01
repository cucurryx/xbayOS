#ifndef __FS_FILE_H
#define __FS_FILE_H

#include <stdint.h>
#include <inode.h>
#include <dir.h>    

#define STDIN  0
#define STDOUT 1
#define STDERR 2

//可打开的最大文件数
#define OPEN_FILE_MAX 32

#define USER_FILE_TABLE_START 3


typedef struct __file file;
typedef enum __bitmap_type bitmap_type;


//文件结构，构成文件表
struct __file {
    //文件操作的偏移地址
    uint32_t fd_offset;

    //文件操作标识
    uint32_t fd_flag;

    //文件对应inode
    inode_t *fd_inode;
};

enum __bitmap_type {
    INODE_BITMAP,
    BLOCK_BITMAP
};


int32_t file_table_alloc();

int32_t fd_install(int32_t ft_idx);

int32_t inode_bm_alloc(partition *part);
int32_t block_bm_alloc(partition *part);

void bitmap_sync(partition *part, uint32_t idx, uint8_t type);

int32_t create_file(struct __dir_t *parent, char *filename, uint8_t flag);


#endif // !__FS_FILE_H