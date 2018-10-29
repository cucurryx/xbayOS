#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

//中断处理程序入口地址
typedef void* intr_handler;

void idt_init();

#endif // ! __KERNEL_INTERRUPT_H
