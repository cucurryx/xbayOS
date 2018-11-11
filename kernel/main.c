#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"
#include "list.h"
#include "thread.h"
#include "interrupt.h"

void *start(void *arg);

int main() {
    put_str("I am kernel\n");
    init_all();
    thread_start("threa1", 16, start, (void*)"thread1 ");
    thread_start("thread2", 16, start, (void*)"thread2 ");
    intr_enable();

    while (true) {
        asm volatile("cli");
        put_str("main ");
        asm volatile("sti");
    }

    while (1);
}


void *start(void *arg) {
    char *s = (char*)arg;
    while (true) {
        asm volatile("cli");
        put_str(s);
        asm volatile("sti");
    }
}