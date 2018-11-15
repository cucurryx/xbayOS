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

void *start(void *arg);
sem_t sem;
mutex_t mutex;

extern io_queue_t keyboard_buffer;

int main() {
    init_all();
    thread_start("threa1", 16, start, (void*)"1_");
    thread_start("thread2", 16, start, (void*)"2_");
    intr_enable();

    while (true) {
        // console_put_str("main ");
        // char c = io_queue_getchar(&keyboard_buffer);
        // console_put_char(c);
    }

    while (1);
}


void *start(void *arg) {
    char *s = (char*)arg;
    
    while (true) {
        char c = io_queue_getchar(&keyboard_buffer);
        console_put_str(s);
        console_put_char(c);
    }
}