#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H

#include "stdint.h"

// character print function, just like putchar in C.
void put_char(uint8_t c);

// string print function, just like puts in C.
void put_str(const char* s);

// integer(hexical) print function 
void put_int(uint32_t x);

// set the position of cursor
void set_cursor(uint32_t x);
#endif
