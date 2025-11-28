#include <stdbool.h>
#include <stdint.h>
#include "lib/string.c"
#include "kernel/video/video.c"
#include "ramfs/ramfs.c"
#include "kernel/memory_manager/memory_manager.h"
#include "lib/inout.h"
#include "kernel/IDT/IDT.h"
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_HEADER_MAGIC, 
    MULTIBOOT_HEADER_FLAGS,
    CHECKSUM,
};

unsigned int stack[4096];


#define SYSCALL(syscal_number, arg1, arg2) ({ \
    long ret; \
    asm volatile ( \
        "int $0x80" \
        : "=a"(ret) \
        : "a"(syscal_number), "b"(arg1), "c"(arg2) \
        : "memory"); \
    ret; \
})


void disable_cursor(){
    outb(0x3d4 , 0x0a);
    outb(0x3d5 , 0x20);
}

void shutdown(){
    outw(0x0604 , 0x2000);
    while(1){
        asm volatile ("hlt");
    }
}
void reboot(){
    uint8_t temp;
    do {
        temp = inb(0x64);
    }
    while (temp & 0x02);
    outb(0x64 , 0xfe);
    while(1){
        asm volatile ("hlt");
    }
}

void system(){
    clear_screen();
    default_color.foreground = VGA_FG_GREEN;
    print_string("  _____   _____   _   _   _    _  __   __     \n");
    print_string(" / ____| |_   _| | \\ | | | |  | | \\ \\ / /  \n");
    print_string("| (___     | |   |  \\| | | |  | |  \\ V /    \n");
    print_string(" \\___ \\    | |   | . ` | | |  | |   > <     \n");
    print_string(" ____) |  _| |_  | |\\  | | |__| |  / . \\    \n");
    print_string("|_____/  |_____| |_| \\_|  \\____/  /_/ \\_\\ \n");
    default_color.foreground = VGA_FG_WHITE;
    print_string("===========================================   \n");
    print_string("OS : SINUX\n");
    print_string("Kernel : Sinux 0.01\n");
    print_string("Memory Size : ");
    char size[20];
    itoa(memory_size / (1024 * 1024), size);
    print_string(size);
    print_string(" MB\n");
    print_string("Free Memory : ");
    char free_size[20];
    itoa(total_free_pages * 4096 / (1024 * 1024), free_size);
    print_string(free_size);
    print_string(" MB\n");
    print_string("===========================================\n");
}

void help() {
    default_color.foreground = VGA_FG_WHITE;
    print_string("Available Commands:\n");
    print_string("===================================\n");
    print_string("shutdown   - Power off the system\n");
    print_string("reboot     - Reboot the system\n");
    print_string("clear      - Clear the screen\n");
    print_string("system     - Display system information\n");
    print_string("ls         - List files in the RAMFS\n");
    print_string("help       - Show this help message\n");
    print_string("===================================\n");
}

multiboot_info_t* mbi_global;

void kernel_main(multiboot_info_t* mbi) {
    mbi_global = mbi;

    disable_cursor();
    init_ramfs(mbi);
    memory_manager_init(mbi);
    init_IDT();

    asm volatile("sti"); // enable interrupts

    clear_screen(0x0f);

    bool on = true;
    char user[15];
    default_color.foreground = VGA_FG_WHITE;
    print_string("================================> [  ");
    default_color.foreground = VGA_FG_LIGHT_GREEN;
    print_string("SINUX");
    default_color.foreground = VGA_FG_WHITE;
    print_string("  ] <=================================");

    print_string("Username : ");
    vin(user);

    clear_screen();

    while(on){
        char command[160];
        default_color.foreground = VGA_FG_LIGHT_GREEN;
        print_string("[ ");
        default_color.foreground = VGA_FG_LIGHT_CYAN;
        print_string(user);
        default_color.foreground = VGA_FG_WHITE;
        print_string(" @ ");
        default_color.foreground = VGA_FG_RED;
        print_string("SINUX ");
        default_color.foreground = VGA_FG_LIGHT_GREEN;
        print_string("] > ");
        default_color.foreground = VGA_FG_WHITE;
        vin(command);

        if(strcmp(command , "shutdown") == 0){
            shutdown();
        }
        else if(strcmp(command , "reboot") == 0){
            reboot();
        }
        else if(strcmp(command , "clear") == 0){
            clear_screen();
        }
        else if(strcmp(command , "system") == 0){
            system();
        }else if(strcmp(command, "ls") == 0){
            print_char('\n');
            int file_index = 0;
            while (file_index < 256 && ramfs_header[file_index].used) {
                char size[12];
                itoa(ramfs_header[file_index].end - ramfs_header[file_index].start, size);
                print_string(ramfs_header[file_index].name);
                print_string("  Size: ");
                print_string(size);
                print_char('\n');
                file_index++;
            }
        }else if(strcmp(command , "help") == 0){
            help();
        }else if(strcmp(command , "modules") == 0){
            print_string("Modules Count: ");
            char modcount[10];
            itoa(mbi->mods_count , modcount);
            print_string(modcount);
            print_char('\n');
            for(uint32_t i = 0; i < mbi->mods_count; i++){
                multiboot_module_t* mod = (multiboot_module_t*)(uintptr_t)(mbi->mods_addr + i * sizeof(multiboot_module_t));
                print_string("Module ");
                char modnum[10];
                itoa(i , modnum);
                print_string(modnum);
                print_string(" Start: ");
                char modstart[20];
                itoa(mod->mod_start , modstart);
                print_string(modstart);
                print_string(" End: ");
                char modend[20];
                itoa(mod->mod_end , modend);
                print_string(modend);
                print_char('\n');
            }
        }else if(strcmp(command, "syscal") == 0){
            SYSCALL(0, 0, 0);
        }else{
            print_string("Unknown command: \n");
            print_string(command);
        }
    }
}
__attribute__((naked)) void _start() {
    asm volatile (
        "mov $stack + 4096, %esp\n"
        "push %ebx\n"
        "call kernel_main\n"
        "cli\n"
        "hlt\n"
    );
}