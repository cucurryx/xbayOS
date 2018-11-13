#ifndef __THREAD_LOCK_H
#define __THREAD_LOCK_H

#include "stdint.h"
#include "list.h"
#include "thread.h"

typedef struct __mutex_t mutex_t;
typedef struct __sem_t sem_t;

struct __sem_t {
    uint8_t value;
    list waiters;
};


struct __mutex_t {
    sem_t sem;
    task_struct *holder;
};


void sem_init(sem_t *sem, uint8_t value);
void sem_up(sem_t *sem);
void sem_down(sem_t *sem);

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif // !__THREAD_LOCK_H