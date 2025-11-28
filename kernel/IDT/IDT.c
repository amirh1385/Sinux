#include <stdint.h>
#include "../../lib/inout.h"
#include "IDT.h"
#include "keyboard/ps2_keyboard.c"
#include "syscal/syscal.c"

struct IDT_entry IDT[256];

void set_IDT_entry(int num, uint32_t base, uint16_t selector, uint8_t type_attr) {
    IDT[num].offset_low  = base & 0xFFFF;          // 16 بیت پایین آدرس
    IDT[num].selector    = selector;               // سگمنت کد (معمولاً 0x08)
    IDT[num].zero        = 0;                      // همیشه صفر
    IDT[num].type_attr   = type_attr;              // نوع گیت (0x8E برای interrupt gate)
    IDT[num].offset_high = (base >> 16) & 0xFFFF;  // 16 بیت بالای آدرس
}

// پورت‌های PIC
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x11
#define ICW4_8086    0x01

// تابع remap کردن PIC
void pic_remap(int offset1, int offset2) {
    uint8_t a1, a2;

    // ذخیره ماسک‌های فعلی
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // شروع راه‌اندازی PICها
    outb(PIC1_COMMAND, ICW1_INIT);
    outb(PIC2_COMMAND, ICW1_INIT);

    // تنظیم offset جدید
    outb(PIC1_DATA, offset1); // معمولاً 0x20
    outb(PIC2_DATA, offset2); // معمولاً 0x28

    // اتصال Master و Slave
    outb(PIC1_DATA, 4); // IRQ2 به Slave وصل است
    outb(PIC2_DATA, 2);

    // تنظیم حالت 8086
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // بازگرداندن ماسک‌ها
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

extern void isr_default_handler();
extern void irq0_handler();
extern void irq1_keyboard_handler();
extern void syscal_handler();
extern void gpf();
extern void init_gdt();
void init_IDT() {
    init_gdt();

    struct IDT_ptr idtp;
    idtp.limit = (sizeof(struct IDT_entry) * 256) - 1;
    idtp.base  = (uint32_t)&IDT;

    pic_remap(0x20, 0x28);
    for(int i = 0; i < 256; i++){
        set_IDT_entry(i, (uint32_t)isr_default_handler, 0x08, 0x8E);
    }

    set_IDT_entry(32, (uint32_t)irq0_handler, 0x08, 0x8E);
    set_IDT_entry(0x21, (uint32_t)irq1_keyboard_handler, 0x08, 0x8E);
    set_IDT_entry(13, (uint32_t)gpf, 0x08, 0x8E);
    set_IDT_entry(0x80, (uint32_t)syscal_handler, 0x08, 0x8E);

    asm volatile("lidt %0" : : "m"(idtp));
}