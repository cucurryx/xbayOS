#include "syscall_init.h"
#include "syscall.h"
#include "stdint.h"
#include "thread.h"
#include "console.h"

#define SYSCALL_NR 32
typedef void* syscall;

syscall syscall_table[SYSCALL_NR];

//注册系统调用号对应的内核处理函数
void syscall_init() {
    syscall_table[SYS_GETPID] = (void*)sys_getpid;
    syscall_table[SYS_WRITE] = (void*)sys_write;
}

//系统实现的getpid
uint16_t sys_getpid() {
    return get_thread_pid(running_thread());
}

uint32_t sys_write(char *s) {
    console_put_str(s);
    return strlen(s);
}