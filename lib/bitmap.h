#ifndef __LIB_BITMAP_H
#define __LIB_BITMAP_H

#include "stdint.h"

//length是bits指向内存的byte数
//bits指向存储bitmap的内存
struct bitmap {
    uint32_t length;
    uint8_t *bits;
};

typedef struct bitmap bitmap;

void bitmap_init(bitmap *bm, uint32_t len);
bool bitmap_has(bitmap *bm, uint32_t position);
int  bitmap_scan(bitmap *bm, uint32_t cnt);
void bitmap_clear(bitmap *bm, uint32_t position);
void bitmap_set(bitmap *bm, uint32_t position);

#endif // !__LIB_BITMAP_H