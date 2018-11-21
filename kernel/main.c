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

int test_val = 0;

uint32_t pid_1 = 1, pid_2 = 1;

int main() {
    init_all();
    process_execute(prog1, "prog1");
    process_execute(prog2, "prog2");


    intr_enable();

    thread_start("threa1", 16, start, (void*)"1_");
    thread_start("thread2", 16, start, (void*)"2_");

    while (1);
}

// time interrupt -> schedule -> switch_to -> kthread -> start_process -> prog

void prog1() {
    pid_1 = getpid();
    while (1);
}

void prog2() {
    pid_2 = getpid();
    while (1);
}

void *start(void *arg) {
    console_put_str("the pid of prog1:");
    console_put_int(pid_1);
    console_put_char('\n');
    console_put_str("the pid of prog2:");
    console_put_int(pid_2);
    while (1);
}