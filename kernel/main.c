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
#include "stdio.h"


void *start1(void *arg);
void *start2(void *arg);
void prog1();
void prog2();
void prog3();
void prog4();

uint32_t pid_1 = -1, pid_2 = -1, pid_3 = -1, pid_4 = -1;

int main() {
    init_all();
    // process_execute(prog1, "prog1");
    // process_execute(prog2, "prog2");
    // process_execute(prog3, "prog3");
    // process_execute(prog4, "prog4");

    thread_start("threa1", 16, start1, (void*)"1_");
    thread_start("thread2", 16, start2, (void*)"2_");
    
    intr_enable();



    while (1);
}

// time interrupt -> schedule -> switch_to -> kthread -> start_process -> prog

void prog1() {
    for (int i = 0; i < 100; ++i) {
        // printf("prog1 hello,");
        printf("prog1:%x, %d <- %s", i, i, "test");
        printf("%c", '\n');
    }
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

void *start1(void *arg) {

    void *addr = sys_malloc(33);
    console_put_str("thread a, sys_malloc(33), addr is 0x");
    console_put_int((uint32_t)addr);
    console_put_char('\n');

    while (1);
}

void *start2(void *arg) {

    void *addr = sys_malloc(33);
    console_put_str("thread b, sys_malloc(33), addr is 0x");
    console_put_int((uint32_t)addr);
    console_put_char('\n');

    while (1);
}