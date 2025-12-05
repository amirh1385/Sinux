#include <stdint.h>
#include <multiboot.h>
#include <arch/x86/mm/pmm.h>
#include <arch/x86/mm/vmm.h>
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
    1,
    1024,
    768,
    32
};

uint32_t stack[4096*4];

void kernel_entry(multiboot_info_t *mbi){
    uart_init();

    pmm_init(mbi);
    vmm_init();

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