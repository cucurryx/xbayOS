#ifndef __DEVICE_TIMER_H
#define __DEVICE_TIMER_H

#include <stdint.h>

//初始化定时计数器8253
void timer_init();

void sleep_by_msecond(uint32_t ms);
void sleep_by_second(uint32_t s);


#endif //!__DEVICE_TIMER_H