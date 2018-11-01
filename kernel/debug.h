#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

#define PANIC(...)                                                  \
    do {                                                            \
        panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__);      \
    } while (0);                                                     

#ifndef NODEBUG
    #define ASSERT(CONDITION)       \
        do {                        \
            if (CONDITION) {        \
                /* do nothing*/     \
            } else {                \
                PANIC(#CONDITION);  \
            }                       \
        } while (0);               
#else
    #define ASSERT(CONDITION) ((void)0)
#endif // !NODEBUG

void panic_spin(const char* filename, int line, const char* func, const char* condition);

#endif // !__KERNEL_DEBUG_H