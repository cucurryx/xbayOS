#ifndef __PROC_TSS_H
#define __PROC_TSS_H
#include "thread.h"

void tss_init();
void set_tss_esp0(task_struct *thread);

#endif // !__PROC_TSS_H