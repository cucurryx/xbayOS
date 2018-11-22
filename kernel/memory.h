#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"
#include "list.h"

//不同大小内存块种类数量
#define MEM_BLOCK_DESC_CNT 7

typedef struct __virtual_addr virtual_addr;
typedef enum __pool_flags pool_flags;
typedef struct __mem_block mem_block;
typedef struct __mem_block_desc mem_block_desc;

//virtual address pool
struct __virtual_addr {
    bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

struct __mem_block {
    list_node block_node;
};

struct __mem_block_desc {
    list block_list;
    uint32_t block_size;
    uint32_t blocks_cnt;
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
void mem_block_init(mem_block_desc *descs);
void *sys_malloc(uint32_t size);

#endif // !__KERNEL_MEMORY_H