#include "interrupt.h"

#include "global.h"
#include "io.h"
#include "stdint.h"
#include "print.h"

#define IDT_DESC_CNT 0x21 //支持的中断数

//8259A主片(master)的控制端口和数据端口
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21

//8259A从片(slave)的控制端口和数据端口
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

//中断门描述符
struct gate_desc {
    uint16_t func_offset_low_word;      //中断处理程序在目标段内的偏移量的低16位
    uint16_t selector;                  //中断处理程序目标代码段描述符选择子
    uint8_t  dcount;                    //未使用的bits，置为0即可
    uint8_t  attribute;                 //中断门描述符属性，包括P位、DPL、S位和type
    uint16_t func_offset_high_word;     //中断处理程序在目标段内的偏移量的高16位
};

static void make_idt_desc(struct gate_desc* gdesc_ptr, uint8_t attr, intr_handler function);
static void pic_init();

static struct gate_desc idt[IDT_DESC_CNT];
extern intr_handler intr_entry_table[IDT_DESC_CNT];

//创建中断门描述符
//参数1：指向新创建的中断门描述符的指针
//参数2：属性
//参数3：中断处理程序
static void make_idt_desc(struct gate_desc* gdesc_ptr, uint8_t attr, intr_handler function) {
    gdesc_ptr->func_offset_low_word = (uint32_t)function & 0x0000ffff;
    gdesc_ptr->selector = SELECTOR_K_CODE;
    gdesc_ptr->dcount = 0;
    gdesc_ptr->attribute = attr;
    gdesc_ptr->func_offset_high_word = ((uint32_t)function >> 16) & 0xffff0000;
}

//初始化中断描述符表
static void idt_desc_init() {
    for (int i = 0; i < IDT_DESC_CNT; ++i) {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
}

static void pic_init() {
    //初始化主片
    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);
    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    //初始化从片
    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);
    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);

    //打开主片上的IR0(时钟产生的中断)
    outb(PIC_M_DATA, 0xfe);
    outb(PIC_S_DATA, 0xff);
}

void idt_init() {
    idt_desc_init();    //初始化中断描述符表
    pic_init();         //初始化程序中断控制器

    //加载idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
}