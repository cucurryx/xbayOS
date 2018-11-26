#include <timer.h>

#include <print.h>
#include <io.h>
#include <interrupt.h>
#include <thread.h>
#include <debug.h>
#include <global.h>
#include <printk.h>

#define IRQ0_FREQUENCY   100 //频率
#define INPUT_FREQUENCY  1193180
#define COUNTER0_VALUE   (INPUT_FREQUENCY / IRQ0_FREQUENCY)
#define COUNTER0_PORT    0x40
#define COUNTER0_NO      0
#define COUNTER_MODE     2
#define RW_LATCH         3
#define PIT_CONTROL_PORT 0x43
#define TICKS_PER_SECOND 100000

//内核态和用户态总共的CPU时间
uint32_t total_times;


//设置频率，将操作的计数器号、读写锁属性、计数器模式写入模式控制寄存器
static void set_frequency(uint8_t port, uint16_t no,uint16_t rw, 
                          uint16_t mode, uint16_t value) {
    outb(PIT_CONTROL_PORT, (uint8_t)(no << 6 | rw << 4 | mode << 1));
    outb(port, (uint8_t)value);
    outb(port, (uint8_t)value >> 8);
}

static void time_intr_handler() {
    ASSERT(intr_get_status() == INTR_OFF);

    task_struct *curr_thread = running_thread();

    ASSERT(curr_thread->stack_magic_num == 0x19980312);

    ++curr_thread->elapsed_time;
    ++total_times;

    //如果当前线程时间片耗尽，则进行调度
    if (curr_thread->ava_time == 0) {
        schedule();
    } else {
        --curr_thread->ava_time;
    }
}

//按照ticks来睡眠，如果当前时间没有到时间，那么挂起当前线程
static void sleep_by_ticks(uint32_t t) {
    uint32_t start = total_times;
    while (start + t > total_times) {
        thread_yield();
    }
}

//以毫秒为单位来睡眠
void sleep_by_msecond(uint32_t ms) {
    uint32_t ticks = DIV_ROUND_UP(ms * TICKS_PER_SECOND, 1000);
    sleep_by_ticks(ticks);
}

//以秒为单位来睡眠
void sleep_by_second(uint32_t s) {
    uint32_t ticks = s * TICKS_PER_SECOND;
    sleep_by_ticks(ticks);
}

void timer_init() {
    set_frequency(COUNTER0_PORT, COUNTER0_NO, RW_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    intr_set_handler(0x20, time_intr_handler);
}
