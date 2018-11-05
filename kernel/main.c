#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"

int main() {
    put_str("I am kernel\n");
    init_all();

    bitmap bm;
    uint8_t a[10];

    bm.bits = a;
    bitmap_init(&bm, 10);

    for (int i = 0; i < 10; ++i) {
        bitmap_set(&bm, i);
    }

    int start = bitmap_scan(&bm, 20);
    for (int i = 0; i < 20; ++i) {
        bitmap_set(&bm, i+start);
    }
    for (int i = 0; i < 8*10; ++i) {
        put_int(i);
        put_char(' ');
        put_int(bitmap_has(&bm, i));
        put_char(' ');
    }

    while (1);
}