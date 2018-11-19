#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"

typedef struct __virtual_addr virtual_addr;
typedef enum __pool_flags pool_flags;

//virtual address pool
struct __virtual_addr {
    bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

enum __pool_flags {
    PF_KERNEL = 1,
    PF_USER = 2
};

#define PG_P_1 1
#define PG_P_0 0
#define PG_RW_R 0   //R/W属性，读/执行
#define PG_RW_W 2   //R/W属性，读/写/执行
#define PG_US_S 0   //U/S属性，系统级
#define PG_US_U 4   //U/S属性，用户级

void mem_init();

void *malloc_page(pool_flags type, uint32_t page_cnt);
void *get_kern_pages(uint32_t page_cnt);
void *get_user_pages(uint32_t page_cnt);
void *get_page(pool_flags type, uint32_t vaddr);
uint32_t vaddr_to_phy(uint32_t vaddr);

#endif // !__KERNEL_MEMORY_H