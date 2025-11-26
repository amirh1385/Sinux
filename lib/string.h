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

void itoa(int num, char* str) {
    int i = 0;
    bool isNegative = false;

    // اگر عدد منفی باشه، باید علامت منفی هم ذخیره بشه
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    // تبدیل عدد به رشته
    do {
        str[i++] = (num % 10) + '0'; // گرفتن باقی‌مانده و تبدیل به کاراکتر
        num = num / 10;  // تقسیم عدد بر ۱۰
    } while (num != 0);

    // اضافه کردن علامت منفی در صورت نیاز
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';  // افزودن null terminator در انتهای رشته

    // معکوس کردن رشته (چون اعداد از آخر به اول اضافه می‌شدن)
    int start = 0;
    int end = i - 1;
    while (start < end) {
        // جا به جایی کاراکترها
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

#endif // STRING_H