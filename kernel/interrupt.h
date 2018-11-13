#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include "stdint.h"

//中断处理程序入口地址
typedef void* intr_handler;

void idt_init();

enum intr_status {INTR_OFF, INTR_ON};

typedef enum intr_status intr_stat;

intr_stat intr_get_status();
intr_stat intr_set_status(intr_stat status);
intr_stat intr_enable();
intr_stat intr_disable();
void intr_set_handler(uint8_t vec_no, intr_handler handler);


#define INTERRUPT_DISABLE(status)                  \
    do {                                           \
        status = intr_disable();                   \
    } while (0);

#define INTERRUPT_RESTORE(status)                  \
    do {                                           \
        intr_set_status(status);                   \
    } while (0);
    
#endif // ! __KERNEL_INTERRUPT_H
