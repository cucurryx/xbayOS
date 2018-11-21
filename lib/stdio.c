#include "stdio.h"
#include "string.h"
#include "syscall.h"

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

static void itoa(char **target, uint32_t var, uint8_t base) {
    uint8_t digits[32];
    int8_t cnt = 0;
    while (var != 0) {
        digits[cnt++] = var % base;
        var /= base;
    }

    --cnt;
    while (cnt >= 0) {
        if (digits[cnt] > 9) {
            **target = (char)('a' + digits[cnt] - 10);
        } else {
            **target = (char)('0' + digits[cnt]);
        }
        ++(*target);
        --cnt;
    }
}
//将format中占位符用ap中参数实际值替换，结果值放在str中返回
uint32_t vsprintf(char *str, const char *format, va_list args) {
    const char *f = format;
    char *p = str, *s;
    int32_t var;

    while (*f != '\0') {
        if (*f == '%') {
            switch (*(++f)) {
                case 'c':
                    var = va_arg(args, char);
                    (*p++) = (char)var;
                    break;
                case 'd':
                    var = va_arg(args, int);
                    if (var < 0) {  //negative
                        var = -var; 
                        (*p++) = '-';
                    }
                    itoa(&p, var, 10);
                    break;
                case 's':
                    s = va_arg(args, char*);
                    p = strcpy(p, s);
                    break;
                case 'x':
                    //函数参数放置在栈中，从栈中读取args
                    var = va_arg(args, int);
                    itoa(&p, var, 16);
                    break;
                case '%':
                    (*p++) = '%';
                default:
                    break;
            }
        } else {
            *p = *f;
            ++p;
        }
        ++f;
    }
    return strlen(str);
}

//实现格式化打印
uint32_t printf(const char *format, ...) {
    va_list args;
    char str[1024];

    va_start(args, format);
    memset(str, 0, 1024);
    vsprintf(str, format, args);
    va_end(args);
    return write(str);
}