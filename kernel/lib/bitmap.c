#include "bitmap.h"

#include "debug.h"
#include "string.h"

#define BITS_TO_BYTE_FLOOR(nbits) ((nbits) / 8)
#define BITS_TO_BYTE_CEIL(nbits) ((nbits + 7) / 8)

void bitmap_zero(uint8_t* map, uint32_t nbits) {
    ASSERT(map != NULL);
    size_t size = BITS_TO_BYTE_CEIL(nbits);
    memset(map, 0, size);
}

void bitmap_fill(uint8_t* map, uint32_t nbits) {
    ASSERT(map != NULL);
    size_t size = BITS_TO_BYTE_CEIL(nbits);
    memset(map, 0xFF, size);
}

void bitmap_and(uint8_t* map, uint8_t* src1, uint8_t* src2, uint32_t nbits) {
    ASSERT(map != NULL);
    int i;
    int nbyte = BITS_TO_BYTE_CEIL(nbits);
    for (i = 0; i < nbyte; i++) {
        map[i] = src1[i] & src2[i];
    }
}

void bitmap_or(uint8_t* map, uint8_t* src1, uint8_t* src2, uint32_t nbits) {
    ASSERT(map != NULL);
    int i;
    int nbyte = BITS_TO_BYTE_CEIL(nbits);
    for (i = 0; i < nbyte; i++) {
        map[i] = src1[i] | src2[i];
    }
}

int bitmap_empty(uint8_t* map, uint32_t nbits) {
    ASSERT(map != NULL);
    int i;
    int nbyte = BITS_TO_BYTE_CEIL(nbits);
    for (i = 0; i < nbyte; i++) {
        if (map[i]) return 0;
    }
    return 1;
}

int bitmap_full(uint8_t* map, uint32_t nbits) {
    ASSERT(map != NULL);
    return !bitmap_empty(map, nbits);
}

void bitmap_set(uint8_t* map, uint32_t pos, uint32_t nbits) {
    ASSERT(map != NULL);
    int i;
    for (i = 0; i < nbits; i++) {
        *(map + BITS_TO_BYTE_FLOOR(pos + i)) |= (1 << (7 - (pos + i) % 8));
    }
}

void bitmap_clear(uint8_t* map, uint32_t pos, uint32_t nbits) {
    ASSERT(map != NULL);
    int i;
    for (i = 0; i < nbits; i++) {
        *(map + BITS_TO_BYTE_FLOOR(pos + i)) &= ~(1 << (7 - (pos + i) % 8));
    }
}

int bitmap_test(uint8_t* map, uint32_t pos) {
    ASSERT(map != NULL);
    return *(map + BITS_TO_BYTE_FLOOR(pos)) & (1 << (7 - pos % 8));
}

int bitmap_find_next_zero_area(uint8_t* map, uint32_t map_size, uint32_t start,
                               uint32_t len) {
    ASSERT(map != NULL);
    int flag;
    uint32_t i;
    while (start + len < map_size) {
        if (bitmap_test(map, start)) {
            start++;
            continue;
        }
        flag = 1;
        for (i = 1; i < len; i++) {
            if (bitmap_test(map, start + i)) {
                flag = 0;
                break;
            }
        }
        if (flag == 1) return start;
        start += i + 1;
    }
    return -1;
}

int bitmap_scan_and_set(uint8_t* map, uint32_t map_size, uint32_t start,
                        uint32_t len) {
    int index = bitmap_find_next_zero_area(map, map_size, start, len);
    if (index != -1) {
        bitmap_set(map, index, len);
    }
    return index;
}

