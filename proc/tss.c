#include "tss.h"
#include "stdint.h"
#include "global.h"
#include "thread.h"
#include "memory.h"
#include "console.h"

typedef struct __tss_t tss_t;

struct __tss_t {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt_selector;
    uint32_t trace;
    uint32_t io_base;
};

//一个CPU只有一个固定的TSS
static tss_t tss;

//设置TSS中的esp0值为线程thread的内核栈
void set_tss_esp0(task_struct *thread) {
    tss.esp0 = (uint32_t)(thread) + PAGE_SIZE;
}

//传入参数，根据参数构造响应的GDT描述符
static gdt_desc make_gdt_desc(uint32_t *addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high) {
    gdt_desc desc;
    uint32_t desc_base = (uint32_t)addr;
    desc.limit_low_word = limit & 0x0000ffff;
    desc.base_low_word = desc_base & 0x0000ffff;
    desc.base_high_byte = (desc_base & 0x00ff0000) >> 16;
    desc.attr_low_byte = (uint8_t)attr_low;
    desc.limit_high_attr_high = (uint8_t)(((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high));
    desc.base_high_byte = desc_base >> 24;
    return desc;
}

//初始化TSS
//在GDT中加入TSS descriptor，code segement descriptor of DPL3, data segment descriptor of DPL3
//lgdt加载GDT，然后ltr加载TSS选择子进tr寄存器
void tss_init() {
    uint32_t tss_size = sizeof(tss_t);
    memset(&tss, 0, tss_size);
    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;

    gdt_desc *gdt = (gdt_desc*)0xc0000900;
    gdt[4] = make_gdt_desc((uint32_t*)&tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);
    gdt[5] = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    gdt[6] = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    //32位基址 | 16位 gdt size
    uint64_t gdt_operand = ((8 * 7 - 1) | ((uint64_t)(uint32_t)0xc0000900 << 16));
    asm volatile ("lgdt %0" : : "m" (gdt_operand));
    asm volatile ("ltr %w0" : : "r" (SELECTOR_TSS));
}
