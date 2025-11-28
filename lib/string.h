#ifndef STRING_H
#define STRING_H

#include <stddef.h>  // برای استفاده از size_t
#include <stdbool.h>

// مقایسه دو رشته
int strcmp(const char *str1, const char *str2);

// کپی کردن یک رشته به دیگری
char *strcpy(char *dest, const char *src);

// تعیین طول رشته
size_t strlen(const char *str);

void itoa(int num, char* str);

#endif // STRING_H