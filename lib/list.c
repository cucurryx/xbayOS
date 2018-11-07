#include "list.h"
#include "interrupt.h"

#define INTERRUPT_DISABLE(status)                  \
    do {                                           \
        status = intr_disable();                   \
    } while (0);

#define INTERRUPT_RESTORE(status)                  \
    do {                                           \
        intr_set_status(status);                   \
    } while (0);

//初始化双向链表，head和tail互相指向
//链表设计为初始head, tail两个节点，然后插入的数据都在两者之间
void list_init(list *l) {
    l->head->next = l->tail;
    l->head->prev = NULL;
    l->tail->prev = l->head;
    l->tail->next = NULL;
}

//在target节点前插入节点node
void list_insert_before(list_node *target, list_node *node) {
    intr_stat status;
    INTERRUPT_DISABLE(status)

    target->prev->next = node;
    node->next = target;
    node->prev = target->prev;
    target->prev = node;

    INTERRUPT_RESTORE(status)
}

//插入节点node到链表头
void list_push_front(list *l, list_node *node) { 
    list_insert_before(l->head->next, node);
}

//插入节点node到链表尾
void list_push_back(list *l, list_node *node) {
    list_insert_before(l->tail, node);
}

//从链表中移除节点node
void list_remove(list_node *node) {
    intr_stat status;
    INTERRUPT_DISABLE(status)

    node->prev->next = node->next;
    node->next->prev = node->prev;

    INTERRUPT_RESTORE(status)
}

//计算链表l的长度
uint32_t list_size(list *l) {
    uint32_t result = 0;
    list_node *curr = l->head->next;
    while (curr != l->tail) {
        ++result;
        curr = curr->next;
    }
    return result;
}

//移除链表l的第一个节点
list_node *list_pop_front(list *l) {
    list_node *node = l->head->next;
    if (node == l->tail) {
        return NULL;
    }
    list_remove(node);
    return node;
}

//移除链表l的最后一个节点
list_node *list_pop_back(list *l) {
    list_node *node = l->tail->prev;
    if (node == l->head) {
        return NULL;
    }
    list_remove(node);
    return node;
}

//遍历链表，找到第一个能够使func(node, arg)返回true的节点，然后返回该节点
list_node *list_traversal(list *l, function func, int arg) {
    list_node *node = l->head->next;
    while (node != l->tail) {
        if (func(node, arg) == true) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

bool list_exist(list *l, list_node *node) {
    list_node *curr = l->head->next;
    while (curr != l->tail) {
        if (curr == node) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

bool list_empty(list *l) {
    return l->head->next == l->tail ? true : false;
}