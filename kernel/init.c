#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"

//初始化所有模块
void init_all() {
    put_str("init_all begin\n");
    idt_init();
    timer_init();
    put_str("init_all ok\n");
}
