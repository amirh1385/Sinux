#include <stdint.h>
#include "../lib/string.h"
#include "../lib/multiboot_info.h"

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
} multiboot_module_t;

struct FileEntry {
    char name[32];    
    uint32_t start;   
    uint32_t end;     
    uint8_t used; 
};

multiboot_module_t* modules;

struct FileEntry* ramfs_header;

void print_module_name(const char* name, int row) {
    volatile char* video = (char*) 0xB8000;
    video += row * 80 * 2;
    int col = 0;
    
    while (*name && col < 80) {
        *video++ = *name++;
        *video++ = 0x0F;
        col++;
    }
}

void init_ramfs(multiboot_info_t* mbi){
    print_string("Searching for RamFS module...\n");
    if (mbi->mods_count == 0) {
        print_string("No modules found!\n");
        return;
    }

    modules = (multiboot_module_t*)(uintptr_t)(mbi->mods_addr);
    ramfs_header = (struct FileEntry*)(uintptr_t)(modules[0].mod_start);
    print_string("RamFS loaded.\n");
}