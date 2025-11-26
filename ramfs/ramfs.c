#include <stdint.h>

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
    uint32_t child_count; // فقط برای فولدر: تعداد فرزندان
    uint32_t child_index; // فقط برای فولدر: index اولین فرزند در آرایه header
};

multiboot_module_t* modules;

extern void vout(const char* str, int row, char color);

struct RamFS_Entry* ramfs_header;

void init_ramfs(multiboot_info_t* mbi){
    modules = (multiboot_module_t*)mbi->mods_addr;
}