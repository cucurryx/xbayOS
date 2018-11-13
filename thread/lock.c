#include "lock.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"

void sem_init(sem_t *sem, uint8_t value) {
    sem->value = value;
    list_init(&sem->waiters);
}

void sem_up(sem_t *sem) {
    intr_stat state;
    INTERRUPT_DISABLE(state);
    
    if (list_empty(&sem->waiters) == false) {
        list_node *node = list_pop_front(&sem->waiters);
        task_struct *next_thread = node_to_task(node, GENERAL_LIST_NODE);
        thread_unblock(next_thread);
    }
    ++sem->value;
    ASSERT(sem->value > 0);
    
    INTERRUPT_RESTORE(state);
}

void sem_down(sem_t *sem) {
    intr_stat state;
    INTERRUPT_DISABLE(state);
    
    //如果信号量已经为0，就阻塞
    while (sem->value == 0) {
        task_struct *curr_thread = running_thread();
        ASSERT(list_exist(&sem->waiters, &curr_thread->gene_list_tag) == false);
        list_push_back(&sem->waiters, &curr_thread->gene_list_tag);
        thread_block(BLOCKED);
    }
    --sem->value;
    ASSERT(sem->value >= 0);

    INTERRUPT_RESTORE(state);
}

void mutex_init(mutex_t *mutex) {

}

void mutex_lock(mutex_t *mutex) {

}

void mutex_unlock(mutex_t *mutex) {

}
