#include "io_queue.h"
#include "debug.h"

void io_queue_init(io_queue_t *q) {
    sem_init(&q->slots, BUF_SIZE);
    sem_init(&q->items, 0);
    sem_init(&q->mutex, 1);
    q->front = 0;
    q->rear = 0;
}

bool io_queue_full(io_queue_t *q) {
     sem_down(&q->mutex);
     bool full = (q->front + 1) % BUF_SIZE == q->rear;
     sem_up(&q->mutex);
     return full;
}

char io_queue_getchar(io_queue_t *q) {
    sem_down(&q->items);
    sem_down(&q->mutex);
    char c = q->buffer[q->front];
    q->front = (q->front + 1) % BUF_SIZE;
    sem_up(&q->mutex);
    sem_up(&q->slots);
    return c;
}

void io_queue_putchar(io_queue_t *q, char c) {
    sem_down(&q->slots);
    sem_down(&q->mutex);
    q->buffer[q->rear] = c;
    q->rear = (q->rear + 1) % BUF_SIZE;
    sem_up(&q->mutex);
    sem_up(&q->items);
}
