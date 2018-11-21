#ifndef __PROC_SYSCALL_INIT_H
#define __PROC_SYSCALL_INIT_H

#include "stdint.h"

void syscall_init();

uint16_t sys_getpid();
uint32_t sys_write(char *s);

#endif // !__PROC_SYSCALL_INIT_H