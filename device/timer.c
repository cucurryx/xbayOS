#include "timer.h"

#include "print.h"
#include "io.h"

#define IRQ0_FREQUENCY   100 //频率
#define INPUT_FREQUENCY  1193180
#define COUNTER0_VALUE   (INPUT_FREQUENCY / IRQ0_FREQUENCY)
#define COUNTER0_PORT    0x40
#define COUNTER0_NO      0
#define COUNTER_MODE     2
#define RW_LATCH         3
#define PIT_CONTROL_PORT 0x43

//设置频率，将操作的计数器号、读写锁属性、计数器模式写入模式控制寄存器
static void set_frequency(uint8_t port, uint16_t no,uint16_t rw, 
                          uint16_t mode, uint16_t value) {
    outb(PIT_CONTROL_PORT, (uint8_t)(no << 6 | rw << 4 | mode << 1));
    outb(port, (uint8_t)value);
    outb(port, (uint8_t)value >> 8);
}

void timer_init() {
    set_frequency(COUNTER0_PORT, COUNTER0_NO, RW_LATCH, COUNTER_MODE, COUNTER0_VALUE);
}
