#ifndef __LIB_LIST_H
#define __LIB_LIST_H

#include "stdint.h"

typedef struct __list_node list_node;
typedef struct __list list;
typedef bool (function)(list_node*, int arg);

struct __list_node {
    list_node *prev;
    list_node *next;
};

struct __list {
    list_node head;
    list_node tail;
};

void list_init(list *l);
void list_insert_before(list_node *before, list_node *node);
void list_push_front(list *l, list_node *node);
void list_push_back(list *l, list_node *node);
void list_remove(list_node *node);

uint32_t list_size(list *l);

list_node *list_pop_front(list *l);
list_node *list_pop_back(list *l);
list_node *list_traversal(list *l, function func, int arg);

bool list_exist(list *l, list_node *node);
bool list_empty(list *l);

#endif // !__LIB_LIST_H