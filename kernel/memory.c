#include "memory.h"
#include "print.h"

//page size为4KB
#define PAGE_SIZE (1 << 12)

//0xc009f000是内核主线程栈顶，所以可以用4个页的内存来作为bitmap
//0xc009e000是内核主线程的PCB
//一个页的bitmap对应 4K * 4K * 8 = 128MB的内存，所以该系统最大支持512MB
#define MEM_BITMAP_BASE 0xc009a000

//内核从0xc0000000起，在高1GB虚拟地址
//需要跨过低端1MB地址
#define K_HEAP_START 0xc0100000

// kernel memory pool and user memory pool
pool kern_pool, user_pool;

//allocate 
virtual_addr kern_vaddr;

static void mem_pool_init(uint32_t all_mem) {
    put_str("  mem_pool_init start\n");
    uint32_t page_table_size = PAGE_SIZE * 256;
    uint32_t used_mem = page_table_size + 0x100000;    //0x100000低端1MB内存
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_pages = free_mem / PAGE_SIZE;
    uint32_t kern_free_pages = all_free_pages / 2;
    uint32_t user_free_pages = all_free_pages - kern_free_pages;

    uint32_t kbm_length = kern_free_pages / 8;
    uint32_t ubm_length = user_free_pages / 8;

    // kernel pool start address and user pool start address
    uint32_t kp_start = used_mem;
    uint32_t up_start = kp_start + kern_free_pages * PAGE_SIZE;

    kern_pool.phy_addr_start = kp_start;
    kern_pool.pool_size = kern_free_pages * PAGE_SIZE;
    kern_pool.pool_bitmap.length = kbm_length;
    kern_pool.pool_bitmap.bits = (uint8_t*)MEM_BITMAP_BASE;

    user_pool.phy_addr_start = up_start;
    user_pool.pool_size = user_free_pages * PAGE_SIZE;
    user_pool.pool_bitmap.length = ubm_length;
    user_pool.pool_bitmap.bits = (uint8_t*)(MEM_BITMAP_BASE + kbm_length);

    put_str("  kernel pool bitmap_start:");
    put_int((int)kern_pool.pool_bitmap.bits);
    put_str("  kernel pool phy_addr_start:");
    put_int((int)kern_pool.phy_addr_start);
    put_str("\n");

    put_str("  used pool bitmap start:");
    put_int((int)user_pool.pool_bitmap.bits);
    put_str("  user pool phy_addr_start:");
    put_int((int)user_pool.phy_addr_start);
    put_str("\n");

    bitmap_init(&kern_pool.pool_bitmap, kern_pool.pool_bitmap.length);
    bitmap_init(&user_pool.pool_bitmap, user_pool.pool_bitmap.length);

    // 内核虚拟地址和内核内存池大小一致
    kern_vaddr.vaddr_bitmap.length = kbm_length;
    kern_vaddr.vaddr_bitmap.bits = (uint8_t*)(MEM_BITMAP_BASE + kbm_length + ubm_length);
    bitmap_init(&kern_vaddr.vaddr_bitmap, kern_vaddr.vaddr_bitmap.length);

    put_str("mempool init done\n");
}

//entry of memory management
void mem_init() {
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00)); //已经在loader中计算得出，放在0xb00
    mem_pool_init(mem_bytes_total);
    put_str("mem_init done\n");
}