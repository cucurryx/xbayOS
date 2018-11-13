#ifndef __THREAD_LOCK_H
#define __THREAD_LOCK_H

#include "stdint.h"
#include "list.h"
#include "thread.h"

typedef struct __mutex_t mutex_t;
typedef struct __sem_t sem_t;

struct __sem_t {
    uint8_t value;          //信号量的值
    list waiters;           //阻塞在该信号量上的线程
};

struct __mutex_t {
    sem_t sem;              //信号量
    task_struct *holder;    //该互斥锁的持有线程
    uint8_t recursion;      //递归层数，用于防止递归加锁
};

void sem_init(sem_t *sem, uint8_t value);
void sem_up(sem_t *sem);
void sem_down(sem_t *sem);

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif // !__THREAD_LOCK_H