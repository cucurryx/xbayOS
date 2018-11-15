#ifndef __DEVICE_IO_QUEUE_H
#define __DEVICE_IO_QUEUE_H

#include "lock.h"
#include "thread.h"
#include "stdint.h"

typedef struct __io_queue_t io_queue_t;

#define BUF_SIZE 64

// producer-comsumer模型
struct __io_queue_t {
    sem_t slots;                //可用空间
    sem_t items;                //可用数据
    sem_t mutex;                //互斥锁保证互斥访问
    char buffer[BUF_SIZE];      //缓冲区
    int32_t front;              //队列头
    int32_t rear;               //队列尾
};

void io_queue_init(io_queue_t *q);
bool io_queue_full(io_queue_t *q);
char io_queue_getchar(io_queue_t *q);
void io_queue_putchar(io_queue_t *q, char c);


#endif // !__DEVICE_IO_QUEUE_H