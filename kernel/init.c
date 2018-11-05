#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"

//初始化所有模块
void init_all() {
    put_str("init_all begin\n");
    idt_init();
    timer_init();
    mem_init();
    put_str("init_all ok\n");
}
