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
mutex_t mutex;

int main() {
    put_str("I am kernel\n");
    init_all();
    thread_start("threa1", 16, start, (void*)"thread1 ");
    thread_start("thread2", 16, start, (void*)"thread2 ");
    intr_enable();

    sem_init(&sem, 1);
    mutex_init(&mutex);

    while (true) {
        mutex_lock(&mutex);
        mutex_lock(&mutex);
        put_str("main ");
        mutex_unlock(&mutex);
        mutex_unlock(&mutex);
    }

    while (1);
}


void *start(void *arg) {
    char *s = (char*)arg;
    
    while (true) {
        mutex_lock(&mutex);
        put_str(s);
        mutex_unlock(&mutex);
    }
}