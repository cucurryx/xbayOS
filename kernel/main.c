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

void *start(void *arg);
void prog();
int test_val = 0;

int main() {
    init_all();

    process_execute(prog, "prog1");
    // process_execute(prog, "prog2");

    thread_start("threa1", 16, start, (void*)"1_");
    // thread_start("thread2", 16, start, (void*)"2_");
    intr_enable();


    while (1);
}

// time interrupt -> schedule -> switch_to -> kthread -> start_process -> prog

void prog() {
    while (true) {
        ++test_val;
    }
}

void *start(void *arg) {
    char *s = (char*)arg;
    while (true) {
        console_put_str("0x");
        console_put_int(test_val);
    }
}