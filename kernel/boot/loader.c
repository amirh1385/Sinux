#include <stdint.h>
#include <multiboot.h>
#include <arch/x86/mm/pmm.h>
#include <arch/x86/mm/vmm.h>
#include <arch/x86/mm/heap.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/gdt.h>
#include <log.h>

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000007
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_HEADER_MAGIC,
    MULTIBOOT_HEADER_FLAGS,
    CHECKSUM,
    0,
    0,
    0,
    0,
    0,
    1,    // type 0=vbe 1=text mode
    1024, // width
    768,  // height
    32    // depth
};

uint32_t stack[4096*4];

void kernel_entry(multiboot_info_t *mbi){
    uart_init();

    pmm_init(mbi);
    vmm_init();
    heap_init();

    gdt_init();
    idt_init();

    while(1){
        asm volatile("hlt");
    }
}

__attribute__((naked)) void _start() {
    asm volatile (
        "mov $stack + 4096, %esp\n"
        "push %ebx\n"
        "call kernel_entry\n"
        "cli\n"
        "hlt\n"
    );
}