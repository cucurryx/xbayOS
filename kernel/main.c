#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"
#include "list.h"
#include "thread.h"
#include "interrupt.h"
#include "lock.h"
#include "console.h"
#include "io_queue.h"
#include "process.h"
#include "syscall.h"

void *start(void *arg);
void prog1();
void prog2();
void prog3();
void prog4();

uint32_t pid_1 = -1, pid_2 = -1, pid_3 = -1, pid_4 = -1;

int main() {
    init_all();
    process_execute(prog1, "prog1");
    process_execute(prog2, "prog2");
    process_execute(prog3, "prog3");
    process_execute(prog4, "prog4");


    intr_enable();

    // thread_start("threa1", 16, start, (void*)"1_");
    thread_start("thread2", 16, start, (void*)"2_");

    while (1);
}

// time interrupt -> schedule -> switch_to -> kthread -> start_process -> prog

void prog1() {
    write("this is prog1");
    while (1);
}

void prog2() {
    write("this is prog2");
    while (1);
}

void prog3() {
    write("this is prog3");
    while (1);
}

void prog4() {
    write("this is prog4");
    while (1);
}

void *start(void *arg) {

    // while (1) {
    //     console_put_str("the pid of prog1:");
    //     console_put_int(pid_1);
    //     console_put_char('\n');
    //     console_put_str("the pid of prog2:");
    //     console_put_int(pid_2);
    //     console_put_char('\n');
    //     console_put_str("the pid of prog3:");
    //     console_put_int(pid_3);
    //     console_put_char('\n');
    //     console_put_str("the pid of prog4:");
    //     console_put_int(pid_4);
    //     console_put_char('\n');
    //     console_put_char('\n');
    // }


    while (1);
}