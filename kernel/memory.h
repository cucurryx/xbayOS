#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"

//virtual address pool
struct __virtual_addr {
    bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

//physical address pool
struct __pool {
    bitmap pool_bitmap;
    uint32_t phy_addr_start;
    uint32_t pool_size;
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

typedef struct __virtual_addr virtual_addr;
typedef struct __pool pool;
typedef enum __pool_flags pool_flags;

extern pool kern_pool, user_pool;
void mem_init();

void *malloc_page(pool_flags type, uint32_t page_cnt);
void *get_kern_pages(uint32_t page_cnt);

#endif // !__KERNEL_MEMORY_H