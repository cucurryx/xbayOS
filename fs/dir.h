#ifndef __FS_DIR_H
#define __FS_DIR_H

#include <stdint.h>

#define FILENAME_LEN 64

typedef struct __dir_entry dir_entry;
typedef enum __entry_type entry_type;

enum __entry_type {
    ET_FILE,
    ET_DIRECTORY
};

//目录项
struct __dir_entry {
    uint32_t inode_no;
    char name[FILENAME_LEN];
    entry_type type;

};

#endif // !__FS_DIR_H