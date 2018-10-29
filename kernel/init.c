#include "init.h"
#include "print.h"
#include "interrupt.h"

void init_all() {
    idt_init();
}