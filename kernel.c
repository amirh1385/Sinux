#include <stdbool.h>
#include <stdint.h>
#include "ramfs/ramfs.c"
#include "kernel/memory_manager/memory_manager.h"
#include "lib/string.h"
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
#define VIDEO_MEM 0xB8000
#define WIDTH 80
#define HEIGHT 25
#define PORT_KEYBOARD 0x60
#define PORT_STATUS 0x64
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_HEADER_MAGIC, 
    MULTIBOOT_HEADER_FLAGS,
    CHECKSUM,
};

unsigned int stack[4096];
const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u',
    'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',
    'l', ';', '\'', '`', 0, '\\', 'z', 'x',
    'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, '*', 0, ' '
};
int row_offsets[25] = {0};
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

void disable_cursor(){
    outb(0x3d4 , 0x0a);
    outb(0x3d5 , 0x20);
}
void clear_screen(char color) {
    volatile char* video = (char*) VIDEO_MEM;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        *video++ = ' ';
        *video++ = color;    
    }
    for (int i = 0; i < HEIGHT; i++) {
        row_offsets[i] = 0;
    }
}
extern void shift_screen_up();
void vout(const char* str, int row, char color) {
    if(row >= HEIGHT){
        shift_screen_up();
        row = HEIGHT - 1;
    }
    volatile char* video = (char*) VIDEO_MEM;
    int offset = row_offsets[row];
    video += row * WIDTH * 2 + offset;
    while (*str && offset < WIDTH * 2) {
        *video++ = *str++;
        *video++ = color;
        offset += 2;
    }
    row_offsets[row] = offset;
}
void vin(char* buffer, char color, int row) {
    volatile char* video = (char*) VIDEO_MEM;
    int offset = row_offsets[row];
    video += row * WIDTH * 2 + offset;
    int index = 0;
    while (1) {
        if (inb(PORT_STATUS) & 1) {
            uint8_t sc = inb(PORT_KEYBOARD);
            char c = (sc < sizeof(scancode_to_ascii)) ? scancode_to_ascii[sc] : 0;
            if (c) {
                if (c == '\n') {
                    buffer[index] = '\0';
                    break;
                }
                else if (c == '\b') {
                    if (index > 0) {
                        index--;
                        offset -= 2;
                        video -= 2;
                        *video++ = ' ';
                        *video++ = color;
                        video -= 2;
                    }
                }
                else {
                    buffer[index++] = c;
                    *video++ = c;
                    *video++ = color;
                    offset += 2;
                }
            }
        }
    }
    row_offsets[row] = offset;
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

extern int numcom;
void shift_screen_up(){
    for (int row = 1; row < HEIGHT; row++) {
        volatile char* src = (char*) VIDEO_MEM + row * WIDTH * 2;
        volatile char* dest = (char*) VIDEO_MEM + (row - 1) * WIDTH * 2;
        for (int col = 0; col < WIDTH * 2; col++) {
            *dest++ = *src++;
        }
        row_offsets[row - 1] = row_offsets[row];
    }
    volatile char* last_row = (char*) VIDEO_MEM + (HEIGHT - 1) * WIDTH * 2;
    for (int col = 0; col < WIDTH * 2; col++) {
        *last_row++ = ' ';
        *last_row++ = 0x0F;
    }
    row_offsets[HEIGHT - 1] = 0;
    numcom--;
}

void system(){
    clear_screen(0x0f);
    vout("  _____   _____   _   _   _    _  __   __     " , 0 , 0x0a);
    vout(" / ____| |_   _| | \\ | | | |  | | \\ \\ / /  " , 1 , 0x0a);
    vout("| (___     | |   |  \\| | | |  | |  \\ V /    " , 2 , 0x0a);
    vout(" \\___ \\    | |   | . ` | | |  | |   > <     " , 3 , 0x0a);
    vout(" ____) |  _| |_  | |\\  | | |__| |  / . \\    " , 4 , 0x0a);
    vout("|_____/  |_____| |_| \\_|  \\____/  /_/ \\_\\ " , 5 , 0x0a);
    vout("==========================================="    , 6 , 0x0f);
    vout("OS : SINUX" , 7 , 0x0f);
    vout("Kernel : Sinux 0.01" , 8 , 0x0f);
    vout("Memory Size : " , 9 , 0x0f);
    char size[20];
    itoa(memory_size / (1024 * 1024), size);
    vout(size , 9 , 0x0f);
    vout(" MB" , 9 , 0x0f);
    vout("Free Memory : " , 10 , 0x0f);
    char free_size[20];
    itoa(total_free_pages * 4096 / (1024 * 1024), free_size);
    vout(free_size , 10 , 0x0f);
    vout(" MB" , 10 , 0x0f);
    vout("==========================================="    , 11 , 0x0f);
}

extern int numcom;
void help() {
    vout("Available Commands:" , numcom , 0x0f);
    vout("===================================" , numcom+1 , 0x0f);
    vout("shutdown   - Power off the system" , numcom+2 , 0x0f);
    vout("reboot     - Reboot the system" , numcom+3 , 0x0f);
    vout("clear      - Clear the screen" , numcom+4 , 0x0f);
    vout("system     - Display system information" , numcom+5 , 0x0f);
    vout("ls         - List files in the RAMFS" , numcom+6 , 0x0f);
    vout("help       - Show this help message" , numcom+7 , 0x0f);
    vout("===================================" , numcom+8 , 0x0f);
    numcom += 9;
}

multiboot_info_t* mbi_global;

int numcom = 0;
void kernel_main(multiboot_info_t* mbi) {
    mbi_global = mbi;
    disable_cursor();
    init_ramfs(mbi);
    memory_manager_init(mbi);

    clear_screen(0x0f);

    bool on = true;
    char user[15];
    vout("================================> [  " , 0 , 0x0f);
    vout("SINUX" , 0 , 0x0a);
    vout("  ] <=================================" , 0 , 0x0f);

    vout("Username : " , 1 , 0x0f);
    vin(user , 0x0f , 1);

    clear_screen(0x0f);

    while(on){
        char command[160];
        vout("[ " , numcom , 0x0a);
        vout(user , numcom , 0x0b);
        vout(" @ ", numcom , 0x0f);
        vout("SINUX " , numcom , 0x4);
        vout("] > " , numcom , 0x0a);
        vin(command , 0x0f , numcom);
        numcom++;
        if(strcmp(command , "shutdown") == 0){
            shutdown();
        }
        else if(strcmp(command , "reboot") == 0){
            reboot();
        }
        else if(strcmp(command , "clear") == 0){
            clear_screen(0x0f);
            numcom = 0;
        }
        else if(strcmp(command , "system") == 0){
            system();
        }else if(strcmp(command, "ls") == 0){
            int file_index = 0;
            while (file_index < 256 && ramfs_header[file_index].used) {
                char size[12];
                itoa(ramfs_header[file_index].end - ramfs_header[file_index].start, size);
                vout(ramfs_header[file_index].name, numcom, 0x0f);
                vout("  Size: ", numcom, 0x0f);
                vout(size, numcom, 0x0f);
                file_index++;
                numcom++;
            }
            numcom++;
        }else if(strcmp(command , "help") == 0){
            help();
        }else if(strcmp(command , "modules") == 0){
            vout("Modules Count: " , numcom , 0x0f);
            char modcount[10];
            itoa(mbi->mods_count , modcount);
            vout(modcount , numcom , 0x0f);
            numcom++;
            for(uint32_t i = 0; i < mbi->mods_count; i++){
                multiboot_module_t* mod = (multiboot_module_t*)(uintptr_t)(mbi->mods_addr + i * sizeof(multiboot_module_t));
                vout("Module " , numcom , 0x0f);
                char modnum[10];
                itoa(i , modnum);
                vout(modnum , numcom , 0x0f);
                vout(" Start: " , numcom , 0x0f);
                char modstart[20];
                itoa(mod->mod_start , modstart);
                vout(modstart , numcom , 0x0f);
                vout(" End: " , numcom , 0x0f);
                char modend[20];
                itoa(mod->mod_end , modend);
                vout(modend , numcom , 0x0f);
                numcom++;
            }
            numcom++;
        }
        else{
            vout("Unknown command: " , numcom , 0x0c);
            vout(command , numcom , 0x0f);
            numcom++;
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