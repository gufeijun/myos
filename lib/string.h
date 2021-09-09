#ifndef __LIB_STRING_H_
#define __LIB_STRING_H_
#include "stdint.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned int size_t;

// TODO some functions may get a higher performance using inline assembly

/**
 * @brief Search for the first occurrence of @c in the first @n bytes of @addr
 * @return the matching position or NULL when @c not exists
 */

static inline void* memchr(const void* addr, uint8_t c, size_t n) {
    while (n--) {
        if (*(uint8_t*)addr == c) {
            return (void*)addr;
        }
        addr++;
    }
    return NULL;
}

/**
 * @brief Compare the first @n bytes of @addr1 and @addr2
 * @return <0 when addr1<addr2, >0 when addr1>addr2, =0 when addr=addr2
 */
static inline int memcmp(const void* addr1, const void* addr2, size_t n) {
    size_t i;
    uint8_t* _addr1 = (uint8_t*)addr1;
    uint8_t* _addr2 = (uint8_t*)addr2;
    for (i = 0; i < n; i++) {
        if (*_addr1++ != *_addr2++) break;
    }
    return *(_addr1 - 1) < *(_addr2 - 1) ? -1 : i != n;
}

/**
 * @brief copy @n bytes from @src into @dest
 * @return the pointer to dest
 */
static inline void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* _src = (uint8_t*)src;
    uint8_t* _dst = (uint8_t*)dest;
    while (n--) {
        *_dst++ = *_src++;
    }
    return dest;
}

/**
 * @brief set the the first @n bytes of @addr to the value @c
 * @return the pointer to addr
 */
static inline void* memset(void* addr, uint8_t c, size_t n) {
    uint8_t* _addr = (uint8_t*)addr;
    while (n--) {
        *_addr++ = c;
    }
    return addr;
}

/**
 * @brief get the length of the arguement @str
 * @return length of @str
 */
static inline size_t strlen(const char* str) {
    size_t i = 0;
    while (str[i++])
        ;
    return i - 1;
}

/**
 * @brief concat the two string @dest and @src
 *
 * @param dest should be large enough to contain src
 *
 * @return the pointer to dest
 */
static inline char* strcat(char* dest, const char* src) {
    size_t len1 = strlen(dest);
    size_t len2 = strlen(src);
    memcpy(dest + len1, src, len2 + 1);
    return dest;
}

/**
 * @brief search the first occurence of charactor @c in @str
 * @return the matching position or NULL when @c not exists
 */
static inline char* strchr(const char* str, char c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return NULL;
}

/**
 * @brief search the last occurence of charactor @c in @str
 * @return the matching position or NULL when @c not exists
 */
static inline char* strrchr(const char* str, char c) {
    char* pos = NULL;
    while (*str) {
        if (*str == c) pos = (char*)str;
        str++;
    }
    return pos;
}

/**
 * @brief compare two strings
 * @return <0 is returned when str1<str2, >0 indicates str2<str1, or str1=str2
 */
static inline int strcmp(const char* str1, const char* str2) {
    while (*str1 != 0 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 < *str2 ? -1 : *str1 > *str2;
}

/**
 * @brief  copy string @src to @dest
 * @return the pointer to @dest
 */
static inline char* strcpy(char* dest, const char* src) {
    char* _dest = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return _dest;
}

#endif
