#ifndef __LIB_STRING_H 
#define __LIB_STRING_H

#include <stdint.h>


void memset(void *dest, uint8_t value, uint32_t size);
void memcpy(void *dest, const void *src, uint32_t size);
uint32_t memcmp(const void *a, const void *b, uint32_t size);

char *strcpy(char *dest, const char *src);
uint32_t strlen(const char *s);
uint8_t strcmp(const char *s1, const char *s2);
uint8_t strncmp(const char *s1, const char *s2, uint32_t n);
char *strchr(const char *s, const char c);
char *strrchr(const char *s, const char c);
char *strcat(char *dest, const char *src);
uint32_t strcnt(const char *s, const char c);

#endif // ! __LIB_STRING_H 