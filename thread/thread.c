#include "thread.h"
#include "string.h"
#include "memory.h"
#include "debug.h"

static void kthread(thread_func func, void *arg) {
    func(arg);
}

//初始化task_struct
void thread_init(task_struct *thread, char *name, int prio) {
    ASSERT(thread != NULL);
    memset(thread, 0, sizeof(thread->kstack));
    strcpy(thread->name, name);
    thread->status = RUNNING;
    thread->priority = prio;
    thread->stack_magic_num = 0x19980312;
    //让kstack指向task_struct指向的页的顶部
    thread->kstack = (uint32_t*)((uint32_t)thread + ((1 << 12)));   
}

//创建线程，对task_struct中的thread_stack进行初始化
void thread_create(task_struct *thread, 
                   thread_func func, 
                   void *func_args) {
    //PCB顶部为中断栈和线程栈
    thread->kstack -= sizeof(intr_stack);    
    thread->kstack -= sizeof(thread_stack);

    //设置线程内核栈
    thread_stack *kthread_stack = (thread_stack*)thread->kstack;
    kthread_stack->ebp = 0;
    kthread_stack->ebx = 0;
    kthread_stack->edi = 0;
    kthread_stack->esi = 0;
    kthread_stack->eip = (uint32_t)kthread; //设置返回地址，ret后会跳转到kthread函数
    kthread_stack->retaddr = 0;
    kthread_stack->func = func;
    kthread_stack->func_arg = func_args;
}

//新建一个线程并开始运行
task_struct *thread_start(char *name, 
                          int prio, 
                          thread_func func, 
                          void *func_arg) {
    //为线程PCB申请一页内存，4KB
    task_struct *thread = (task_struct*)get_kern_pages(1);
    thread_init(thread, name, prio);
    thread_create(thread, func, func_arg);

    asm volatile (
        "movl %0, %%esp; "
        "pop %%ebp;"
        "pop %%ebx;"
        "pop %%edi;"
        "pop %%esi;"
        "ret;"
        :: "g" (thread->kstack)
        : "memory"
    );
    return thread;
}

