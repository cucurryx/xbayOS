#include "lock.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"

//初始化信号量
void sem_init(sem_t *sem, uint8_t value) {
    sem->value = value;
    list_init(&sem->waiters);
}

//递增信号量，需要关闭中断，来满足互斥
//然后根据情况来更新waiter队列，如果有阻塞线程，那么解除其阻塞状态
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

//递减互斥量，需要关闭中断，满足互斥
//如果信号量减为0，则阻塞调用该函数的线程，并加入waiter队列
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

//mutex就是一个二元信号量
void mutex_init(mutex_t *mutex) {
    mutex->holder = NULL;
    mutex->recursion = 0;
    sem_init(&mutex->sem, 1);
}

//互斥锁加锁，判断是否为重复加锁，如果重复加锁，那么只需要递增递归数
//否则，递减信号量，并更新holder和recursion
void mutex_lock(mutex_t *mutex) {
    if (mutex->holder != running_thread()) {
        sem_down(&mutex->sem);
        ASSERT(mutex->recursion == 0);
        mutex->holder = running_thread();
        ++mutex->recursion;
    } else {
        ++mutex->recursion;
    }
}

//互斥锁解锁，判断是否需要让出该锁
//如果需要让出，那么更新holder和recursion，递增信号量
//否则，只需要递减递归数
void mutex_unlock(mutex_t *mutex) {
    ASSERT(mutex->holder = running_thread());
    if (mutex->recursion == 1) {
        mutex->holder = NULL;
        mutex->recursion = 0;
        sem_up(&mutex->sem);
    } else {
        --mutex->recursion;
    }
}
