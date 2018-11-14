#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
#include "console.h"

//初始化所有模块
void init_all() {
    idt_init();
    timer_init();
    mem_init();
    thread_init();
    console_init();

    console_put_str("init_all ok\n");
}
