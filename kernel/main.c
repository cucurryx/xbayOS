#include "print.h"
#include "init.h"
#include "debug.h"

int main() {
    put_str("I am kernel\n");
    init_all();
    put_str("init_all done\n");
    asm volatile("sti");    
    while (1);
}