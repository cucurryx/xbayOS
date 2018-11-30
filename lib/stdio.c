#include <stdio.h>
#include <string.h>
#include <syscall.h>

static void itoa(char **target, uint32_t var, uint8_t base) {
    uint8_t digits[32];
    int8_t cnt = 0;

    if (var == 0) {
        digits[cnt++] = 0;
    } else {
        while (var != 0) {
            digits[cnt++] = var % base;
            var /= base;
        }
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
                    p += strlen(s);
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

//实现格式化打印到标准输出
uint32_t printf(const char *format, ...) {
    va_list args;
    char str[1024];

    va_start(args, format);
    memset(str, 0, 1024);
    vsprintf(str, format, args);
    va_end(args);
    return write(str);
}

//实现格式化打印到字符串
uint32_t sprintf(char *s, const char *format, ...) {
    uint32_t len;
    va_list args;
    va_start(args, format);
    len = vsprintf(s, format, args);
    va_end(args);
    return len; 
}