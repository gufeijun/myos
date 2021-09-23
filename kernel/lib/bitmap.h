#ifndef __KERNEL_LIB_BITMAP_H
#define __KERNEL_LIB_BITMAP_H
#include "stdint.h"

// the length of map must be an integer multiple of 8 bits

// set @nbits bits to zero from address of @map
void bitmap_zero(uint8_t* map, uint32_t nbits);

// set @nbits bits from address of @map
void bitmap_fill(uint8_t* map, uint32_t nbits);

// *map = *src1 & *src2
void bitmap_and(uint8_t* map, uint8_t* src1, uint8_t* src2, uint32_t nbits);

// *map = *src1 | *src2
void bitmap_or(uint8_t* map, uint8_t* src1, uint8_t* src2, uint32_t nbits);

// are all bits zero in *map?
int bitmap_empty(uint8_t* map, uint32_t nbits);

// are all bits set in *map?
int bitmap_full(uint8_t* map, uint32_t nbits);

// set bits range from map+pos to map+pos+nbits
void bitmap_set(uint8_t* map, uint32_t pos, uint32_t nbits);

// clear bits range from map+pos to map+pos+nbits
void bitmap_clear(uint8_t* map, uint32_t pos, uint32_t nbits);

// find zero area of @len bits from @map+@start
int bitmap_find_next_zero_area(uint8_t* map, uint32_t map_size, uint32_t start,
                               uint32_t len);

// is bit set in pos?
int bitmap_test(uint8_t* map, uint32_t pos);

int bitmap_scan_and_set(uint8_t* map, uint32_t map_size, uint32_t start,
                        uint32_t len);

#endif
