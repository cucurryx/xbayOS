#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "stdint.h"

typedef enum __task_status task_status;
typedef struct __intr_stack intr_stack;
typedef struct __thread_stack thread_stack;
typedef struct __task_struct task_struct;
typedef void* (*thread_func)(void*);

//进程、线程状态
enum __task_status {
    RUNNING, 
    READY, 
    BLOCKED, 
    WAITING, 
    HANGING, 
    DIED
};

//中断栈，中断发生时保存程序的上下文环境
//发生中断时，外部中断或者软中断，会以此顺序压入内核栈
//该栈在内核栈所在页的最顶端
struct __intr_stack {
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
};

struct __thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    uint32_t eip;       //初始线程eip指向kthread函数，线程切换时指向switch_to函数

    uint32_t retaddr;   //用来占位的过程调用返回地址
    thread_func func;   //线程的start_routine函数
    void *func_arg;     //线程start_routine函数的参数
};

struct __task_struct {
    uint32_t *kstack;    //线程内核栈
    task_status status;       //线程状态
    uint8_t priority;         //线程调度优先级
    char name[16];            //线程名
    uint32_t stack_magic_num; //线程魔数，用于边界检查，防止内核栈溢出覆盖task_struct数据
};

void thread_init(task_struct *thread, char *name, int prio);
void thread_create(task_struct *thread, thread_func func, void *func_args);
task_struct *thread_start(char *name, int prio, thread_func func, void *func_args);

#endif // !__THREAD_THREAD_H
