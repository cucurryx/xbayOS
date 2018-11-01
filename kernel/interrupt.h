#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

//中断处理程序入口地址
typedef void* intr_handler;

void idt_init();

enum intr_status {INTR_OFF, INTR_ON};

typedef enum intr_status intr_stat;

intr_stat intr_get_status();
intr_stat intr_set_status(intr_stat status);
intr_stat intr_enable();
intr_stat intr_disable();

#endif // ! __KERNEL_INTERRUPT_H
