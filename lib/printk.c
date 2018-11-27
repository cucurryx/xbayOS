#include <printk.h>
#include <console.h>

//内核格式化打印函数
void printk(const char *format, ...) {
    va_list args;
    char str[1024];

    va_start(args, format);
    memset(str, 0, 1024);
    vsprintf(str, format, args);
    va_end(args);
    console_put_str(str);
}