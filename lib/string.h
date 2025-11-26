#ifndef STRING_H
#define STRING_H

#include <stddef.h>  // برای استفاده از size_t

// مقایسه دو رشته
int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

// کپی کردن یک رشته به دیگری
char *strcpy(char *dest, const char *src) {
    char *dst = dest;
    while ((*dst++ = *src++)) {
        // کپی کردن کاراکترها
    }
    return dest;
}

// تعیین طول رشته
size_t strlen(const char *str) {
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

#endif // STRING_H