#include "debug.h"
#include "interrupt.h"
#include "print.h"

void panic_spin(const char* filename, int line, 
                const char* func, const char* cond) {
    //关闭中断
    intr_disable();

    put_str("\n\n    ##error##\n");
    put_str("[filename]:  ");
    put_str(filename);
    put_str("\n");
    put_str("[line]:      0x");
    put_int(line);
    put_char('\n');
    put_str("[function]:  ");
    put_str(func);
    put_str("\n");
    put_str("[condtion]:  ");
    put_str(cond);
    put_str("\n");

    //一直自旋
    while(1) { }
}