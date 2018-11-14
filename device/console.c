#include "console.h"
#include "print.h"

//世界上只有一个console
static console cons;

void console_init() {
    mutex_init(&cons.console_lock);
}

void console_lock() {
    mutex_lock(&cons.console_lock);
}

void console_unlock() {
    mutex_unlock(&cons.console_lock);
}

void console_put_str(char *s) {
    console_lock();
    put_str(s);
    console_unlock();
}

void console_put_char(char c) {
    console_lock();
    put_char(c);
    console_unlock();
}

void console_put_int(uint32_t x) {
    console_lock();
    put_str("0x");
    put_int(x);
    console_unlock();
}
