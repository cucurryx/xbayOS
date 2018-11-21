#include "syscall.h"
#include "stdint.h"

#define _syscall0(NUM) ({ int retval; asm volatile ("int $0x80" : "=a" (retval) : "a" (NUM) : "memory"); retval; })
 
#define _syscall1(NUM, ARG1) NULL

#define _syscall2(NUM, ARG1, ARG2) NULL

#define _syscall3(NUM, ARG1, ARG2, ARG3) NULL


uint16_t getpid() {
    return _syscall0(SYS_GETPID);
}