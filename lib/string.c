#include "string.h"
#include "debug.h"

//将dest开始的size个字节设置为value
void memset(void *dest, uint8_t value, uint32_t size)  {
    ASSERT(dest != NULL);

    uint8_t *temp = (uint8_t*)dest;
    for (int i = 0; i < size; ++i) {
        *temp = value;
        ++temp;
    }
}

//将src开始的size个字节拷贝到dest开始的内存中
void memcpy(void *dest, const void *src, uint32_t size) {
    ASSERT(dest != NULL);
    ASSERT(src != NULL);

    uint8_t *d = (uint8_t*)dest, *s = (uint8_t*)src;
    for (int i = 0; i < size; ++i) {
        *d = *s;
        ++d;
        ++s;
    }
}

//比较a和b指向的size字节的数据,如果相等返回0,如果a>b返回1,否则返回-1
uint32_t memcmp(const void *a, const void *b, uint32_t size) {
    ASSERT(a != NULL);
    ASSERT(b != NULL);

    uint8_t *a_ = (uint8_t*)a, *b_ = (uint8_t*)b;
    for (int i = 0; i < size; ++i) {
        if (*a_ > *b_) {
            return 1;
        } else if (*a_ < *b_) {
            return -1;
        } 
        ++a_;
        ++b_;
    }
    return 0;
}

//将字符串src拷贝到dest
char *strcpy(char *dest, const char *src) {
    ASSERT(dest != NULL);
    ASSERT(src != NULL);

    char *d = dest, *s = (char*)src;
    while (*d != '\0') {
        *d = *s;
        ++d;
        ++s;
    }
    return dest;
}

//返回字符串s的长度
uint32_t strlen(const char *s) { 
    uint32_t len = 0;
    if (s == NULL) {
        return len;
    }
    char *p = (char*)s;
    while (*p != '\0') {
        ++p;
    }
    return p - s;
}

//compare两个字符串,如果相等,返回0,如果s1>s2返回1,否则返回-1
uint8_t strcmp(const char *s1, const char *s2) { 
    ASSERT(s1 != NULL);
    ASSERT(s2 != NULL);

    char *a = (char*)s1, *b = (char*)s2;
    while (*a != '\0' && a == b) {
        ++a;
        ++b;
    }
    if (*a < *b) {
        return -1;
    } else if (*a > *b) {
        return 1;
    } else {
        return 0;
    }
} 

//compare两个字符串的前n个字符,如果相等,返回0,如果s1>s2返回1,否则返回-1
uint8_t strncmp(const char *s1, const char *s2, uint32_t n) {
    ASSERT(s1 != NULL);
    ASSERT(s2 != NULL);

    for (int i = 0; i < n; ++i) {
        if (s1[i] > s2[i]) {
            return 1;
        } else if (s1[i] < s2[i]) {
            return -1;
        }
    }
    return 0;
} 

//从左向右查找字符串s中首次出现字符c的地址。如果找到，就返回该字符的地址，否则返回NULL。
char *strchr(const char *s, const char c) { 
    ASSERT(s != NULL);

    char *p = (char*)s;
    while (*p != '\0') {
        if (*p == c) {
            return p;
        }
        ++p;
    }
    return NULL;
} 

//从右向左查找字符串s中首次出现字符c的地址
char *strrchr(const char *s, const char c) { 
    ASSERT(s != NULL);

    char *p = (char*)s, *last_char = NULL;
    while (*p != '\0') {
        if (*p == c) {
            last_char = p;
        }
        ++p;
    }
    return last_char;
} 

//拼接字符串dest和src,dest在前,src在后,返回拼接后字符串的地址
char *strcat(char *dest, const char *src) { 
    ASSERT(dest != NULL);
    ASSERT(src != NULL);

    char *p = dest, *s = (char*)src;
    while (*p != '\0') {
        ++p;
    }
    
    while (*s != '\0') {
        *p = *s;
        ++p;
        ++s;
    }
    *p = '\0';
    return dest;
} 

//返回字符串s中字符c出现的次数
uint32_t strcnt(const char *s, const char c) { 
    ASSERT(s != NULL);
    char *p = (char*)s;

    uint32_t cnt = 0;
    while (*p != '\0') {
        if (*p == c) {
            ++cnt;
        }
        ++p;
    }
    return cnt;
} 