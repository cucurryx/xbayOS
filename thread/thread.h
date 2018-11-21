#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "memory.h"
#include "stdint.h"
#include "list.h"

#define DEFAULT_THREAD_PRIO 32
#define PAGE_SIZE (1 << 12)

typedef enum __task_status task_status;
typedef struct __intr_stack intr_stack;
typedef struct __thread_stack thread_stack;
typedef struct __task_struct task_struct;
typedef void* (*thread_func)(void*);
typedef uint16_t pid_t;

//进程、线程状态
enum __task_status {
    RUNNING, 
    READY, 
    BLOCKED, 
    WAITING, 
    HANGING, 
    DIED
};

typedef enum __node_type {
    GENERAL_LIST_NODE, 
    ALL_LIST_NODE
} node_type;

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
    uint32_t *kstack;         //线程内核栈
    task_status status;       //线程状态
    uint8_t priority;         //线程调度优先级
    pid_t pid;                //进程pid
    uint32_t ava_time;        //剩余可用的CPU时间
    uint32_t elapsed_time;    //该线程已经占用的CPU时间
    char name[16];            //线程名
    list_node gene_list_tag;  //普通链表使用的tag
    list_node all_list_tag;   //专门给all_threads list使用的tag
    uint32_t* page_dir;       //指向页表。如果为线程则为NULL，如果为进程则指向自己的页目录表
    virtual_addr user_vaddr;  //如果是进程，那么是该进程的用户虚拟内存池 
    uint32_t stack_magic_num; //线程魔数，用于边界检查，防止内核栈溢出覆盖task_struct数据
};


task_struct *main_thread;       //主线程的task_struct
list ready_thread_list;         //就绪任务队列
list all_thread_list;           //所有任务队列

void task_struct_init(task_struct *thread, char *name, int prio);
void thread_create(task_struct *thread, thread_func func, void *func_args);
task_struct *thread_start(char *name, int prio, thread_func func, void *func_args);
task_struct *running_thread();
void schedule();
void switch_to(task_struct *curr, task_struct *next);
void thread_init(); 
void thread_block(task_status state);
void thread_unblock(task_struct *thread);
task_struct *node_to_task(list_node *node, node_type type);
pid_t get_thread_pid(task_struct *thread);

#endif // !__THREAD_THREAD_H
