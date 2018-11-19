#ifndef __PROC_PROCESS_H
#define __PROC_PROCESS_H

#include "thread.h"

#define USER_STACK_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8048000

void *start_process(void *prog);
void process_execute(void *prog, char *name);
void process_activate(task_struct *thread);

#endif // !__PROC_PROCESS_H