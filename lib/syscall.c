#include <syscall.h>
#include <stdint.h>

#define _syscall0(NUM)                                          \
    ({                                                          \        
        int retval;                                             \
        asm volatile (                                          \
            "int $0x80"                                         \
            : "=a" (retval)                                     \
            : "a" (NUM)                                         \
            : "memory"                                          \
        );                                                      \
        retval;                                                 \
    })
 
#define _syscall1(NUM, ARG1)                                    \
    ({                                                          \        
        int retval;                                             \
        asm volatile (                                          \
            "int $0x80"                                         \
            : "=a" (retval)                                     \
            : "a" (NUM), "b" (ARG1)                             \
            : "memory"                                          \
        );                                                      \   
        retval;                                                 \
    })

#define _syscall2(NUM, ARG1, ARG2)                              \
    ({                                                          \             
        int retval;                                             \
        asm volatile (                                          \
            "int $0x80"                                         \
            : "=a" (retval)                                     \
            : "a" (NUM), "b" (ARG1), "c" (ARG2)                 \
            : "memory"                                          \
        );                                                      \
        retval;                                                 \
    })


#define _syscall3(NUM, ARG1, ARG2, ARG3)                        \
    ({                                                          \        
        int retval;                                             \
        asm volatile (                                          \
            "int $0x80"                                         \
            : "=a" (retval)                                     \
            : "a" (NUM), "b" (ARG1), "c" (ARG2), "d" (ARG3)     \
            : "memory"                                          \
        );                                                      \
        retval;                                                 \
    })


uint16_t getpid() {
    return _syscall0(SYS_GETPID);
}

uint32_t write(char *s) {
    return _syscall1(SYS_WRITE, s);
}

void *malloc(uint32_t size) {
    return (void*)_syscall1(SYS_MALLOC, size);
}

void free(void *p) {
    _syscall1(SYS_FREE, p);
}