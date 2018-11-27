#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H

#include "stdint.h"

typedef char* va_list;

#define va_start(ap, first)     \
    do {                        \
        ap = (va_list)&first;    \
    } while (0);


#define va_arg(ap, type) *((type*)(ap += 4))

#define va_end(ap)  \
    do {            \
        ap = NULL;  \
    } while (0);

#define va_copy(dest, src)                  \
    do {                                    \
        memcpy(dest, src, sizeof(src));     \
    } while (0);
    
uint32_t vsprintf(char *str, const char *format, va_list ap);
uint32_t printf(const char *format, ...);
uint32_t sprintf(char *s, const char *format, ...);

#endif // !__LIB_STDIO_H