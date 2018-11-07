#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"
#include "list.h"
#include "thread.h"

void *start(void *arg);

int main() {
    put_str("I am kernel\n");
    init_all();

    // void *p1 = get_kern_pages(2);
    thread_start("thread1", 1, start, (void*)"thread1 ");

    while (1);
}


void *start(void *arg) {
    char *s = (char*)arg;
    while (true) {
        put_str(s);
        put_char('\n');
    }
}