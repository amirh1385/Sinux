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

// دریافت دایرکتوری با آدرس (مسیر)
// آدرس مثلاً: "/", "/folder", "/folder/subfolder"
// بازگشت: ایندکس دایرکتوری یا -1 اگر پیدا نشد
int32_t ramfs_find_directory(const char* path) {
    // اگر "/" باشد، فولدر اول (روت) را برگردان
    if (strcmp(path, "/") == 0) {
        return 0;
    }

    // شروع از روت
    uint32_t current_dir = 0;
    const char* p = path;

    // skip اولین "/"
    if (*p == '/') {
        p++;
    }

    // parse کردن هر بخش از مسیر
    while (*p) {
        // پیدا کردن آخر اسم پوشه (تا "/" یا "\0")
        const char* dir_end = p;
        while (*dir_end && *dir_end != '/') {
            dir_end++;
        }

        // طول نام پوشه
        uint32_t name_len = dir_end - p;
        if (name_len == 0) {
            p++;
            continue;
        }

        // جستجو برای این پوشه در فرزندان current_dir
        uint32_t child_idx = ramfs_header[current_dir].child_index;
        int32_t found = -1;

        while (child_idx != 0) {
            // بررسی اگر نام مطابقت دارد
            if (ramfs_header[child_idx].type == 2) {
                // مقایسه نام
                int match = 1;
                for (uint32_t i = 0; i < name_len; i++) {
                    if (ramfs_header[child_idx].name[i] != p[i]) {
                        match = 0;
                        break;
                    }
                }
                if (match && ramfs_header[child_idx].name[name_len] == '\0') {
                    found = child_idx;
                    break;
                }
            }
            child_idx = ramfs_header[child_idx].next_index;
        }

        if (found == -1) {
            return -1; // پوشه پیدا نشد
        }

        current_dir = found;
        p = dir_end;

        // skip "/"
        if (*p == '/') {
            p++;
        }
    }

    return current_dir;
}

// دریافت لیست فایل‌های یک دایرکتوری
// dir_index: ایندکس دایرکتوری (از ramfs_find_directory)
// بازگشت: تعداد فایل‌ها
uint32_t ramfs_list_directory(uint32_t dir_index, struct RamFS_Entry** entries_out) {
    if (dir_index >= 64 || ramfs_header[dir_index].type != 2) {
        return 0; // ایندکس معتبر نیست یا فولدر نیست
    }

    uint32_t child_idx = ramfs_header[dir_index].child_index;
    uint32_t count = 0;

    // شمارش تعداد فایل‌ها
    while (child_idx != 0 && count < 64) {
        entries_out[count] = &ramfs_header[child_idx];
        child_idx = ramfs_header[child_idx].next_index;
        count++;
    }

    return count;
}