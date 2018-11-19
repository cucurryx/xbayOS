#include "memory.h"
#include "print.h"
#include "debug.h"
#include "string.h"
#include "lock.h"

//page size为4KB
#define PAGE_SIZE (1 << 12)

//0xc009f000是内核主线程栈顶，所以可以用4个页的内存来作为bitmap
//0xc009e000是内核主线程的PCB
//一个页的bitmap对应 4K * 4K * 8 = 128MB的内存，所以该系统最大支持512MB
#define MEM_BITMAP_BASE 0xc009a000

//内核从0xc0000000起，在高1GB虚拟地址
//需要跨过低端1MB地址
#define K_HEAP_START 0xc0100000

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

typedef struct __pool pool;

//physical address pool
struct __pool {
    bitmap pool_bitmap;
    uint32_t phy_addr_start;
    uint32_t pool_size;
    mutex_t mutex;
};

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

    mutex_init(&kern_pool.mutex);
    kern_pool.phy_addr_start = kp_start;
    kern_pool.pool_size = kern_free_pages * PAGE_SIZE;
    kern_pool.pool_bitmap.length = kbm_length;
    kern_pool.pool_bitmap.bits = (uint8_t*)MEM_BITMAP_BASE;

    mutex_init(&user_pool.mutex);
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
    kern_vaddr.vaddr_start = K_HEAP_START;
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

//在虚拟内存中申请page_cnt个虚拟页，返回虚拟页的起始地址
static void *vaddr_get(pool_flags type, uint32_t page_cnt) {
    int res_start = 0, bit_position = -1;
    if (type == PF_KERNEL) {
        bit_position = bitmap_scan(&kern_vaddr.vaddr_bitmap, page_cnt);
        if (bit_position != -1) {
            for (uint32_t i = 0; i < page_cnt; ++i) {
                bitmap_set(&kern_vaddr.vaddr_bitmap, bit_position + i);
            }
            res_start = kern_vaddr.vaddr_start + bit_position * PAGE_SIZE;
        } else {
            return NULL;
        }
    } else {
        task_struct *curr_thread = running_thread();
        virtual_addr *user_vaddr = &curr_thread->user_vaddr;
        bit_position = bitmap_scan(&user_vaddr->vaddr_bitmap, page_cnt);
        if (bit_position != -1) {
            for (uint32_t i = 0; i < page_cnt; ++i) {
                bitmap_set(&user_vaddr->vaddr_bitmap, bit_position + i);
            }
            res_start = user_vaddr->vaddr_start + bit_position * PAGE_SIZE;
            ASSERT((uint32_t)res_start < (0xc0000000 - PAGE_SIZE));
        } else {
            return NULL;
        }
    }
    return (void*)res_start;
}

//返回虚拟地址vaddr对应的pte指针
//因为返回的是指针，实际就是一个虚拟地址
//而这个虚拟地址对应的物理地址，就是vaddr对应的页表的物理地址
uint32_t *pte_ptr(uint32_t vaddr) {
    uint32_t *pte = (uint32_t*)(        \
        0xffc00000 +                    \
        ((vaddr & 0xffc00000) >> 10) +  \
        PTE_IDX(vaddr) * 4);
}

//返回虚拟地址vaddr对应pde的指针
uint32_t *pde_ptr(uint32_t vaddr) {
    //最后一个页目录项中存储的是页目录表的物理地址
    //所以，0xfffffxxx访问的实际就是offset个页目录项
    uint32_t *pde = (uint32_t*)((0xfffff000 + PDE_IDX(vaddr) * 4));
    return pde;
}

//在po指向的物理内存池中分配一个物理页
//成功返回页框的物理地址，否则NULL
static void *palloc(pool *po) {
    int bit_position = bitmap_scan(&po->pool_bitmap, 1);
    if (bit_position != -1) {
        bitmap_set(&po->pool_bitmap, bit_position);
        uint32_t page_phyaddr = ((bit_position * PAGE_SIZE) + po->phy_addr_start);
        return (void*)page_phyaddr;
    } else {
        return NULL;
    }
}

//在页表中添加虚拟地址vaddr和物理地址page_phyaddr的映射
//实际是在页表中添加虚拟地址对应的页表项pte，并把物理页的物理地址写入到该pte
static void page_table_add(void *vaddr, void *phyaddr) {
    uint32_t virt_addr = (uint32_t)vaddr, page_phyaddr = (uint32_t)phyaddr;
    uint32_t *pde = pde_ptr(virt_addr), *pte = pte_ptr(virt_addr);

    if (*pde & 0x1) {
        ASSERT(!(*pte & 0x1));
        //只要是创建了页表，pte就应该不存在
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    } else {
        uint32_t pde_phyaddr = (uint32_t)palloc(&kern_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void*)((int)pte & 0xfffff000), 0, PAGE_SIZE);
        ASSERT(!(*pte & 0x1));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

//分配page_cnt个页的空间空间
//成功返回其虚拟地址，否则返回NULL
//分别palloc申请page_cnt个页框，然后将其物理地址和对应的内存池的虚拟地址关联起来
void *malloc_page(pool_flags type, uint32_t page_cnt) {
    ASSERT(page_cnt > 0);
    ASSERT(page_cnt < 3840);

    void *vaddr_start = vaddr_get(type, page_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }

    uint32_t vaddr = (uint32_t)vaddr_start;
    pool *mempool = type == PF_KERNEL ? &kern_pool : &user_pool;

    for (int i = 0; i < page_cnt; ++i) {
        void *page_phyaddr = palloc(mempool);
        if (page_phyaddr == NULL) {
            return NULL;
        }
        page_table_add((void*)vaddr, page_phyaddr);
        vaddr += PAGE_SIZE;
    }
    return vaddr_start;
}

//从内核物理池中申请page_cnt个页的内存
//如果成功，返回其虚拟地址，否则返回NULL
void *get_kern_pages(uint32_t page_cnt) {
    void *vaddr = malloc_page(PF_KERNEL, page_cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, page_cnt * PAGE_SIZE);
    }
    return vaddr;
}

//从用户物理内存池中分配page_cnt个页面的内存
//并返回其虚拟地址
void *get_user_pages(uint32_t page_cnt) {
    mutex_lock(&user_pool.mutex);
    void *vaddr = malloc_page(PF_USER, page_cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, page_cnt * PAGE_SIZE);
    }
    mutex_unlock(&user_pool.mutex);
    return vaddr;
}

//将地址vaddr和type对应的物理地址池中的物理地址相关联
void *get_page(pool_flags type, uint32_t vaddr) {
    pool *mem_pool = (type == PF_KERNEL) ? &kern_pool : &user_pool;
    mutex_lock(&mem_pool->mutex);

    uint32_t bit_position = -1;
    task_struct *curr_thread = running_thread();

    if (type == PF_KERNEL && curr_thread->page_dir == NULL) {
        bit_position = (vaddr - kern_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bit_position > 0);
        bitmap_set(&kern_vaddr.vaddr_bitmap, bit_position);
    } else if (type == PF_USER && curr_thread->page_dir != NULL) {
        bit_position = (vaddr - curr_thread->user_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bit_position > 0);
        bitmap_set(&curr_thread->user_vaddr.vaddr_bitmap, bit_position);
    } else {
        PANIC("neither userspace allocate nor kernel allocate!");
    }

    void *phyaddr = palloc(mem_pool);
    if (phyaddr == NULL) {
        return NULL;
    }
    page_table_add((void*)vaddr, phyaddr);
    mutex_unlock(&mem_pool->mutex);
    return (void*)vaddr;
}

//虚拟地址转化为物理地址
//首先找到vaddr对应的页表，然后加上vaddr低12位的偏移量
uint32_t vaddr_to_phy(uint32_t vaddr) {
    uint32_t *pte = pte_ptr(vaddr);
    return (*pte & 0xfffff000) + (vaddr & 0x00000fff);
}
