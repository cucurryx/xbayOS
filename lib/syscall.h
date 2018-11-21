#ifndef __LIB_SYSCALL_H
#define __LIB_SYSCALL_H

#include "stdint.h"

typedef enum __syscall_nr syscall_nr;

enum __syscall_nr {
    SYS_GETPID = 0,
};

uint16_t getpid();

#endif // !__LIB_SYSCALL_H