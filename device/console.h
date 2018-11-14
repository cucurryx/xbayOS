#ifndef __DEVICE_CONSOLE_H

#include "lock.h"
#include "stdint.h"

typedef struct __console console;

struct __console {
    mutex_t console_lock;
};

void console_init();
void console_lock();
void console_unlock();
void console_put_str(char *s);
void console_put_char(char c);
void console_put_int(uint32_t x);

#endif // !__DEVICE_CONSOLE_H