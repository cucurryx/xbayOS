#include <print.h>
#include <init.h>
#include <debug.h>
#include <string.h>
#include <bitmap.h>
#include <memory.h>
#include <list.h>
#include <thread.h>
#include <interrupt.h>
#include <lock.h>
#include <console.h>
#include <io_queue.h>
#include <process.h>
#include <syscall.h>
#include <stdio.h>
#include <timer.h>

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

    thread_start("threa1", 16, start1, (void*)"1");
    thread_start("thread2", 16, start2, (void*)"2");
    
    intr_enable();



    while (1);
}

// time interrupt -> schedule -> switch_to -> kthread -> start_process -> prog

void prog1() {
    uint32_t size = 11;
    for (int i = 0; i < 7; ++i) {
        printf("[proc1] addr: 0x%x\n", (uint32_t)malloc(size));
        printf("[proc1] addr: 0x%x\n", (uint32_t)malloc(size));
        printf("[proc1] addr: 0x%x\n", (uint32_t)malloc(size));
        printf("[proc1] addr: 0x%x\n", (uint32_t)malloc(size));

        size *= 2;
    }
    while (1);
}

void prog2() {
    uint32_t size = 11;
    for (int i = 0; i < 7; ++i) {
        printf("[proc2] addr: 0x%x\n", (uint32_t)malloc(size));
        printf("[proc2] addr: 0x%x\n", (uint32_t)malloc(size));
        printf("[proc2] addr: 0x%x\n", (uint32_t)malloc(size));
        printf("[proc2] addr: 0x%x\n", (uint32_t)malloc(size));

        size *= 2;
    }
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
    char buf[10];
    for (int i = 0; i < 10; ++i) {
        sleep_by_msecond(1000);
        printf("a:%x  \n", i);
    }
    while (1);
}

void *start2(void *arg) {
    for (int i = 0; i < 10; ++i) {
        sleep_by_second(1);
        printf("b:%x  \n", i);
    }
    while (1);
}