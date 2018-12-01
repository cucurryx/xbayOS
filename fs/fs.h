#ifndef __FS_FS_H
#define __FS_FS_H

#include <stdint.h>
#include <file.h>
#include <dir.h>
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

//路径最大长度
#define PATH_MAXLEN 512

#define O_RDONLY 0x0    //000b
#define O_WRONLY 0x1    //001b
#define O_RDWR   0x2    //010b
#define O_CREAT   0x4    //100b

typedef enum __file_type file_type;
typedef struct __path_record path_record;

enum __file_type {
    FT_UNKOWN,
    FT_FILE,
    FT_DIRECTORY
};

struct __path_record {
    char prev[PATH_MAXLEN];
    struct __dir_t *parent;
    file_type type;
};

void fs_init();

uint32_t path_depth(const char *path);
int32_t sys_open(const char *path, uint8_t flag);



#endif // !__FS_FS_H