#ifndef __FS_DIR_H
#define __FS_DIR_H

#include <stdint.h>
#include <fs.h>
#include <disk.h>


#define FILENAME_LEN 64

#define DIR_BUF_LEN 512

#define DIRECT_BLK_CNT 12
#define BLK_POINTER_SIZE 4



typedef struct __dir_entry dir_entry;
typedef struct __dir_t dir_t;
typedef enum __entry_type entry_type;


enum __entry_type {
    ET_UNKOWN,
    ET_FILE,
    ET_DIRECTORY
};

//目录项
struct __dir_entry {
    uint32_t inode_no;
    char name[FILENAME_LEN];
    entry_type type;
};

//目录
struct __dir_t {
    inode_t *inode;
    uint32_t offset;
    uint8_t dir_buf[DIR_BUF_LEN];
};

void open_root(partition *part);

dir_t *open_dir(partition *part, uint16_t inode_no);
void close_dir(dir_t *dir);
bool dir_exist(partition *part, dir_t *dir, const char *name, dir_entry *e);

void set_entry(char *name, uint32_t inode_no, entry_type type, dir_entry *e);
bool add_entry(dir_t *parent, dir_entry *e, void *buf);


#endif // !__FS_DIR_H