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

typedef struct __virtual_addr virtual_addr;
typedef struct __pool pool;


extern pool kern_pool, user_pool;
void mem_init();

#endif // !__KERNEL_MEMORY_H