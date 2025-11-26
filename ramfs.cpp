#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

// مشخصات آرایه header
struct RamFS_Entry {
    char name[64];       // نام فایل یا پوشه (حداکثر 63 کاراکتر + null)
    uint32_t type;       // 1 = فایل، 2 = پوشه
    uint32_t start;      // فقط برای فایل: offset شروع داده در فایل
    uint32_t end;        // فقط برای فایل: offset پایان داده
    uint32_t next_index; // index فایل/فولدر بعدی (0xFFFFFFFF = آخرین)
    uint32_t child_index; // فقط برای فولدر: index اولین فایل/فولدر فرزند
    uint8_t used;
    uint8_t padding[3];  // برای alignment
};

const size_t HEADER_SIZE = 4096; // اندازه header ثابت
const char* INPUT_DIR = "files";
const char* OUTPUT_FILE = "isodir/boot/ramfs";

std::vector<RamFS_Entry> entries;
std::vector<std::vector<char>> file_data; // برای ذخیره محتوای فایل‌ها
uint32_t current_offset = HEADER_SIZE; // داده‌ها بعد از header شروع می‌شوند

// تابع recursive برای scan فایل‌ها و فولدرها
uint32_t process_directory(const fs::path& dir_path) {
    uint32_t parent_index = entries.size();

    RamFS_Entry folder_entry;
    memset(&folder_entry, 0, sizeof(folder_entry));
    std::string folder_name = dir_path.filename().string();
    strncpy(folder_entry.name, folder_name.c_str(), sizeof(folder_entry.name) - 1);
    folder_entry.type = 2;
    folder_entry.start = 0;
    folder_entry.used = 1;
    folder_entry.end = 0;
    folder_entry.child_index = 0xFFFFFFFF; // هنوز فرزندی نیست
    folder_entry.next_index = 0xFFFFFFFF;
    entries.push_back(folder_entry);

    uint32_t first_child = 0xFFFFFFFF;
    uint32_t last_child = 0xFFFFFFFF;

    for (auto& p : fs::directory_iterator(dir_path)) {
        if (p.is_directory()) {
            uint32_t child_idx = process_directory(p.path());
            
            if (first_child == 0xFFFFFFFF) {
                first_child = child_idx;
            } else {
                entries[last_child].next_index = child_idx;
            }
            last_child = child_idx;
            
        } else if (p.is_regular_file()) {
            RamFS_Entry file_entry;
            memset(&file_entry, 0, sizeof(file_entry));
            std::string name = p.path().filename().string();
            strncpy(file_entry.name, name.c_str(), sizeof(file_entry.name) - 1);
            file_entry.type = 1;

            // خواندن داده فایل
            std::ifstream fin(p.path(), std::ios::binary);
            std::vector<char> data((std::istreambuf_iterator<char>(fin)),
                                    std::istreambuf_iterator<char>());
            fin.close();

            file_entry.start = current_offset;
            file_entry.end = current_offset + data.size();
            file_entry.used = 1;
            file_entry.next_index = 0xFFFFFFFF;
            current_offset += data.size();

            uint32_t file_idx = entries.size();
            entries.push_back(file_entry);
            file_data.push_back(std::move(data));
            
            if (first_child == 0xFFFFFFFF) {
                first_child = file_idx;
            } else {
                entries[last_child].next_index = file_idx;
            }
            last_child = file_idx;
        }
    }

    // به روز رسانی child_index پوشه
    if (first_child == 0xFFFFFFFF) {
        entries[parent_index].child_index = 0; // فولدر خالی است
    } else {
        entries[parent_index].child_index = first_child;
    }

    return parent_index;
}

int main() {
    // scan پوشه اصلی
    fs::path root(INPUT_DIR);
    process_directory(root);

    // نوشتن فایل خروجی
    std::ofstream fout(OUTPUT_FILE, std::ios::binary);
    if (!fout) {
        std::cerr << "Cannot open output file\n";
        return 1;
    }

    // نوشتن header با اندازه ثابت
    std::vector<char> header_buf(HEADER_SIZE, 0);
    size_t entry_size = entries.size() * sizeof(RamFS_Entry);
    if (entry_size > HEADER_SIZE) {
        std::cerr << "Too many files/folders, header overflow!\n";
        return 1;
    }

    memcpy(header_buf.data(), entries.data(), entry_size);
    fout.write(header_buf.data(), HEADER_SIZE);

    // نوشتن داده فایل‌ها پشت header
    for (auto& d : file_data) {
        fout.write(d.data(), d.size());
    }

    fout.close();

    std::cout << "RamFS binary created: " << OUTPUT_FILE << "\n";
    std::cout << "Total entries created: " << entries.size() << "\n";
    for (size_t i = 0; i < entries.size(); i++) {
        std::string indent(entries[i].type == 2 ? 0 : 2, ' ');
        std::string type_str = (entries[i].type == 1) ? "FILE" : "DIR";
        std::cout << indent << "[" << i << "] " << entries[i].name 
                  << " (type: " << type_str;
        if (entries[i].type == 1) {
            std::cout << ", size: " << (entries[i].end - entries[i].start) << " bytes";
        } else {
            std::cout << ", first_child: " << entries[i].child_index;
        }
        std::cout << ", next: " << entries[i].next_index << ")\n";
    }
    return 0;
}