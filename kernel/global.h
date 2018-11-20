#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H

#include "stdint.h"

//关于中断描述符的一些属性定义
#define  RPL0  0
#define  RPL1  1
#define  RPL2  2
#define  RPL3  3

#define  TI_GDT 0
#define  TI_LDT 1

#define  SELECTOR_K_CODE    ((1 << 3) + (TI_GDT << 2) + RPL0)
#define  SELECTOR_K_DATA    ((2 << 3) + (TI_GDT << 2) + RPL0)
#define  SELECTOR_K_STACK   SELECTOR_K_DATA 
#define  SELECTOR_K_GS      ((3 << 3) + (TI_GDT << 2) + RPL0)

#define  SELECTOR_U_CODE    ((5 << 3) + (TI_GDT << 2) + RPL3)
#define  SELECTOR_U_DATA    ((6 << 3) + (TI_GDT << 2) + RPL3)
#define  SELECTOR_U_STACK   ((6 << 3) + (TI_GDT << 2) + RPL3)

//IDT描述符属性
#define  IDT_DESC_P      1 
#define  IDT_DESC_DPL0   0
#define  IDT_DESC_DPL3   3
#define  IDT_DESC_32_TYPE     0xe   // 32位的门
#define  IDT_DESC_16_TYPE     0x6   // 16位的门
#define  IDT_DESC_ATTR_DPL0  ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define  IDT_DESC_ATTR_DPL3  ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)


//GDT描述符属性
#define  GDT_DESC_G_4K          1
#define  GDT_DESC_D_32          1   
#define  GDT_DESC_L             0
#define  GDT_DESC_AVL           0
#define  GDT_DESC_P             1
#define  GDT_DESC_DPL_0         0
#define  GDT_DESC_DPL_1         1
#define  GDT_DESC_DPL_2         2
#define  GDT_DESC_DPL_3         3
#define  GDT_DESC_S_CODE        1
#define  GDT_DESC_S_DATA        1
#define  GDT_DESC_S_SYS         0
#define  GDT_DESC_TYPE_CODE     8   
#define  GDT_DESC_TYPE_DATA     2   
#define  GDT_DESC_TYPE_TSS      9

#define  GDT_ATTR_HIGH            ((GDT_DESC_G_4K << 7) + (GDT_DESC_D_32 << 6) + (GDT_DESC_L << 5) + (GDT_DESC_AVL << 4))
#define  GDT_CODE_ATTR_LOW_DPL3   ((GDT_DESC_P << 7) + (GDT_DESC_DPL_3 << 5) + (GDT_DESC_S_CODE <<4) + GDT_DESC_TYPE_CODE)
#define  GDT_DATA_ATTR_LOW_DPL3   ((GDT_DESC_P << 7) + (GDT_DESC_DPL_3 << 5) + (GDT_DESC_S_CODE <<4) + GDT_DESC_TYPE_DATA)

//TSS描述符属性
#define TSS_DESC_D  0

#define TSS_ATTR_HIGH ((GDT_DESC_G_4K << 7) + (TSS_DESC_D << 6) + (GDT_DESC_L << 5) + (GDT_DESC_AVL << 4) + 0x0)
#define TSS_ATTR_LOW  ((GDT_DESC_P << 7) + (GDT_DESC_DPL_0 << 5) + (GDT_DESC_S_SYS << 4) + GDT_DESC_TYPE_TSS)
#define SELECTOR_TSS  ((4 << 3) + (TI_GDT << 2 ) + RPL0)

//GDT描述符
typedef struct __gdt_desc {
    uint16_t limit_low_word;
    uint16_t base_low_word;
    uint8_t base_mid_byte;
    uint8_t attr_low_byte;
    uint8_t limit_high_attr_high;
    uint8_t base_high_byte;
} gdt_desc;

#define EFLAGS_MBS      (1 << 1)
#define EFLAGS_IF_1     (1 << 9)
#define EFLAGS_IF_0     0
#define EFLAGS_IOPL_3   (3 << 12)
#define EFLAGS_IOPL_0   (0 << 12)

#define DIV_ROUND_UP(x, step) ((x + step - 1) / step)

#endif // !__KERNEL_GLOBAL_H