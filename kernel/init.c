#include <init.h>
#include <print.h>
#include <interrupt.h>
#include <timer.h>
#include <memory.h>
#include <thread.h>
#include <console.h>
#include <keyboard.h>
#include <tss.h>
#include <syscall_init.h>
#include <disk.h>
#include <fs.h>

//初始化所有模块
void init_all() {
    idt_init();
    timer_init();
    mem_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();
    ide_channel_init();
    fs_init();


    console_put_str("init_all ok\n");
}
