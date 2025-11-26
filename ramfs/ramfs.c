#include <stdint.h>
#include "../lib/string.h"

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info_t;

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
} multiboot_module_t;

struct RamFS_Entry {
    char name[64];       // نام فایل یا پوشه (حداکثر 63 کاراکتر + null)
    uint32_t type;       // 1 = فایل، 2 = پوشه
    uint32_t start;      // فقط برای فایل: offset شروع داده در فایل
    uint32_t end;        // فقط برای فایل: offset پایان داده
    uint32_t next_index; // index فایل/فولدر بعدی (0xFFFFFFFF = آخرین)
    uint32_t child_index; // فقط برای فولدر: index اولین فرزند
    uint8_t used;
    uint8_t padding[3];  // برای alignment
};

multiboot_module_t* modules;

extern void vout(const char* str, int row, char color);

struct RamFS_Entry* ramfs_header;

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
    vout("Searching for RamFS module...", 10, 0x0E);
    if (mbi->mods_count == 0) {
        vout("No modules found!", 10, 0x0C);
        return;
    }

    modules = (multiboot_module_t*)(uintptr_t)(mbi->mods_addr);
    ramfs_header = (struct RamFS_Entry*)(uintptr_t)(modules[0].mod_start);
    vout("RamFS loaded.", 11, 0x0A);
}