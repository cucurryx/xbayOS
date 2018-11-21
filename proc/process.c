#include "process.h"
#include "thread.h"
#include "global.h"
#include "memory.h"
#include "lock.h"
#include "interrupt.h"
#include "debug.h"
#include "bitmap.h"
#include "tss.h"
#include "console.h"

//中断退出函数，用来从内核态进入用户态
extern void intr_exit();

//设置当前PCB，然后调用intr_exit进入用户态，启动进程
void *start_process(void *prog) {
    task_struct *curr = running_thread();
    curr->kstack += sizeof(thread_stack);
    intr_stack *proc_context = (intr_stack*)curr->kstack;

    proc_context->edi = 0;
    proc_context->esi =0;
    proc_context->ebp = 0;
    proc_context->esp_dummy = 0;

    proc_context->ebx = 0;
    proc_context->edx = 0;
    proc_context->ecx = 0;
    proc_context->eax = 0;

    proc_context->gs = 0; //不允许访问显存段
    proc_context->ds = SELECTOR_U_DATA;
    proc_context->es = SELECTOR_U_DATA;
    proc_context->fs = SELECTOR_U_DATA;
    proc_context->eip = (uint32_t)prog;
    proc_context->cs = SELECTOR_U_CODE;
    proc_context->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_context->esp = (uint32_t)get_page(PF_USER, USER_STACK_VADDR) + PAGE_SIZE;
    proc_context->ss = SELECTOR_U_DATA;
    
    asm volatile ("movl %0, %%esp;"
                  "jmp intr_exit"
                  :: "g" (proc_context) : "memory");

    return NULL;
}

//创建用户虚拟地址内存池的bitmap
static void create_user_vaddr_bitmap(task_struct *prog) {
    uint32_t length = (0xc0000000 - USER_VADDR_START) / PAGE_SIZE / 8;
    uint32_t bitmap_page_cnt = DIV_ROUND_UP(length, PAGE_SIZE);
    prog->user_vaddr.vaddr_start = USER_VADDR_START;
    prog->user_vaddr.vaddr_bitmap.bits = get_kern_pages(bitmap_page_cnt);
    bitmap_init(&prog->user_vaddr.vaddr_bitmap, length);
}   

//创建用户进程的页目录表
//将虚拟地址空间高1GB映射到内核空间
static void *create_page_dir() {
    uint32_t *page_dir_vaddr = (uint32_t*)get_kern_pages(1);
    if (page_dir_vaddr == NULL) {
        PANIC("get kernel page fail!");
    }
    //复制内核页目录表的高256项
    uint32_t *user_pde_kern_part = (uint32_t*)((uint32_t)page_dir_vaddr + 0x300 * 4);
    uint32_t *kern_pde_kern_part = (uint32_t*)(0xfffff000 + 0x300 * 4);
    memcpy(user_pde_kern_part, kern_pde_kern_part, 1024);
    uint32_t new_page_dir_phy_addr = vaddr_to_phy(page_dir_vaddr);
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}

//创建进程，相比线程的创建，多了用户虚拟内存池和用户进程页表的创建
void process_execute(void *prog, char *name) {
    task_struct *thread = get_kern_pages(1);
    task_struct_init(thread, name, DEFAULT_THREAD_PRIO);
    create_user_vaddr_bitmap(thread);
    thread_create(thread, (thread_func)start_process, prog);
    thread->page_dir = (uint32_t*)create_page_dir();

    intr_stat status;
    INTERRUPT_DISABLE(status);

    ASSERT(list_exist(&all_thread_list, &thread->all_list_tag) == false);
    ASSERT(list_exist(&ready_thread_list, &thread->gene_list_tag) == false);
    list_push_back(&all_thread_list, &thread->all_list_tag);
    list_push_back(&ready_thread_list, &thread->gene_list_tag);

    INTERRUPT_RESTORE(status);
}

//激活进程，分别更新cr3寄存器和tss
//在用户态下使用进程自己的页表，以及保存当前进程的内核栈在tss的esp0中
void process_activate(task_struct *thread) {
    ASSERT(thread != NULL);

    //默认为内核自己的页目录表物理地址
    uint32_t pagedir_phy_addr = 0x100000;

    if (thread->page_dir != NULL) {
        pagedir_phy_addr = vaddr_to_phy((uint32_t)thread->page_dir);
        // set_tss_esp0(thread);
    }
    //更新cr3寄存器
    asm volatile ("movl %0, %%cr3" : : "r" (pagedir_phy_addr) : "memory");

    if (thread->page_dir != NULL) {
        // pagedir_phy_addr = vaddr_to_phy((uint32_t)thread->page_dir);
        set_tss_esp0(thread);
    }
}