#ifndef __LIB_SYSCALL_H
#define __LIB_SYSCALL_H

#include "stdint.h"

typedef enum __syscall_nr syscall_nr;

enum __syscall_nr {
    SYS_GETPID = 0,
    SYS_WRITE  = 1,
    SYS_MALLOC = 3,
    SYS_FREE   = 4
};


uint16_t getpid();
uint32_t write(char *s);
void *malloc(uint32_t size);
void free(void *p);

#endif // !__LIB_SYSCALL_H