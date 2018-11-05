#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"

int main() {
    put_str("I am kernel\n");
    init_all();

    void *p1 = get_kern_pages(2);

    put_str("get kernel page start virtual address is:");
    put_int((uint32_t)p1);

    put_str("\n");
    void *p2 = get_kern_pages(10);
    put_int((uint32_t)p2);

    while (1);
}