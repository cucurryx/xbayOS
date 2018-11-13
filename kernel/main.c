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

void *start(void *arg);
sem_t sem;

int main() {
    put_str("I am kernel\n");
    init_all();
    thread_start("threa1", 16, start, (void*)"thread1 ");
    thread_start("thread2", 16, start, (void*)"thread2 ");
    intr_enable();

    sem_init(&sem, 1);

    while (true) {
        // sem_down(&sem);
        put_str("main ");
        // sem_up(&sem);
    }

    while (1);
}


void *start(void *arg) {
    char *s = (char*)arg;
    
    while (true) {
        // sem_down(&sem);
        put_str(s);
        // sem_up(&sem);
    }
}