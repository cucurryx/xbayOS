#include "bitmap.h"
#include "string.h"
#include "debug.h"

//初始化bitmap，设置其长度，并将所有bits置为0
void bitmap_init(bitmap *bm, uint32_t len) {
    bm->length = len;
    memset((void*)bm->bits, 0, len);
}

//判断position对应的bit是否为1
bool bitmap_has(bitmap *bm, uint32_t position) {
    uint32_t idx = position / 8;
    uint8_t offset = position % 8;
    ASSERT(idx < bm->length);
    return !!((bm->bits[idx]) & (0x1 << offset));
}

int bitmap_scan(bitmap *bm, uint32_t cnt) {
    uint32_t idx = 0, offset = 0;
    while ((bm->bits[idx] == 0xff) && (idx < bm->length)) {
        ++idx;
    }

    //没有可用空间
    if (idx == bm->length) {
        return -1;
    }

    while ((uint8_t)(0x1 << offset) & bm->bits[idx]) {
        ++offset;
    }

    int position = idx * 8 + offset;
    if (cnt == 1) {
        return position;
    }

    uint32_t next_bit = position + 1, count = 1;
    position = -1;
    for (uint32_t left = bm->length * 8 - position; left > 0; --left) {
        if (!(bitmap_has(bm, next_bit))) {
            ++count;
        } else {
            count = 0;
        }
        if (count == cnt) {
            position = next_bit - cnt + 1;
            break;
        }
        ++next_bit;
    }
    return position;
}

void bitmap_clear(bitmap *bm, uint32_t position) {
    uint32_t idx = position / 8;
    uint8_t offset = position % 8;
    ASSERT(idx < bm->length);
    bm->bits[idx] &= ~(0x1 << offset);
}

void bitmap_set(bitmap *bm, uint32_t position) {
    uint32_t idx = position / 8;
    uint8_t offset = position % 8;
    ASSERT(idx < bm->length);
    bm->bits[idx] |= (0x1 << offset);
}