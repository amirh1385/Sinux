#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct IDT_entry {
    uint16_t offset_low;    // 0..15 بیت پایین آدرس تابع سرویس وقفه
    uint16_t selector;      // Segment selector (معمولاً کد سگمنت کرنل)
    uint8_t  zero;          // همیشه صفر
    uint8_t  type_attr;     // نوع گیت و بیت‌های دسترسی (مثلاً 0x8E برای interrupt gate)
    uint16_t offset_high;   // 16..31 بیت بالای آدرس تابع سرویس وقفه
} __attribute__((packed));

struct IDT_ptr {
    uint16_t limit;   // اندازه جدول - 1
    uint32_t base;    // آدرس شروع جدول
} __attribute__((packed));

void init_IDT(void);

void set_IDT_entry(int num, uint32_t base, uint16_t selector, uint8_t type_attr);

#endif // IDT_H