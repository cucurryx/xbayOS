#include "syscall_init.h"
#include "syscall.h"
#include "stdint.h"


#define SYSCALL_NR 32
typedef void* syscall;

syscall syscall_table[SYSCALL_NR];

//注册系统调用号对应的内核处理函数
void syscall_init() {
    syscall_table[SYS_GETPID] = (void*)sys_getpid;
}

//系统实现的getpid
uint16_t sys_getpid() {
    return 0;
}