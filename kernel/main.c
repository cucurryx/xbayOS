#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"
#include "list.h"

int main() {
    put_str("I am kernel\n");
    init_all();

    void *p1 = get_kern_pages(2);

    put_str("get kernel page start virtual address is:");
    put_int((uint32_t)p1);

    put_str("\n");
    void *p2 = get_kern_pages(10);
    put_int((uint32_t)p2);

    list *l;
    list_node head, tail;
    l->head = &head;
    l->tail = &tail;

    list_init(l);
    list_node nodes[10];
    for (int i = 0; i < 10; ++i) {
        list_push_back(l, &nodes[i]);
    }

    put_char('\n');
    put_int(list_size(l));

    for (int i = 0; i < 10; ++i) {
        list_pop_back(l);
    }

    put_char('\n');
    put_int(list_size(l));

    while (1);
}