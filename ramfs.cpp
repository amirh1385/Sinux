#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

struct FileEntry {
    char name[32];    // نام فایل
    uint32_t start;   // آدرس شروع فایل در خروجی
    uint32_t end;     // آدرس پایان فایل
    uint8_t used; // نشان‌دهنده استفاده یا عدم استفاده از ورودی
};

const size_t MAX_FILES = 256;

int main() {
    std::vector<fs::path> files;

    // خواندن تمام فایل‌های پوشه "files"
    for (const auto& entry : fs::directory_iterator("files")) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    if (files.empty()) {
        std::cerr << "هیچ فایلی در پوشه 'files' پیدا نشد.\n";
        return 1;
    }

    // آرایه ثابت با اندازه مشخص
    FileEntry fileTable[MAX_FILES];
    std::memset(fileTable, 0, sizeof(fileTable)); // صفر کردن آرایه

    std::ofstream out("isodir/boot/ramfs", std::ios::binary);
    if (!out) {
        std::cerr << "نمی‌توان فایل خروجی را باز کرد.\n";
        return 1;
    }

    // رزرو فضای آرایه در فایل خروجی
    out.write(reinterpret_cast<char*>(fileTable), sizeof(fileTable));

    uint32_t currentOffset = sizeof(fileTable);

    for (size_t i = 0; i < files.size() && i < MAX_FILES; ++i) {
        std::ifstream in(files[i], std::ios::binary | std::ios::ate);
        if (!in) {
            std::cerr << "نمی‌توان فایل را باز کرد: " << files[i] << "\n";
            return 1;
        }

        std::streamsize size = in.tellg();
        in.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!in.read(buffer.data(), size)) {
            std::cerr << "خواندن فایل با مشکل مواجه شد: " << files[i] << "\n";
            return 1;
        }

        // نوشتن داده فایل به خروجی
        out.write(buffer.data(), size);

        // پر کردن اطلاعات فایل در آرایه
        std::strncpy(fileTable[i].name, files[i].filename().string().c_str(),
                     sizeof(fileTable[i].name) - 1);
        fileTable[i].start = currentOffset;
        fileTable[i].end = currentOffset + static_cast<uint32_t>(size);
        fileTable[i].used = 1;

        currentOffset += static_cast<uint32_t>(size);
    }

    // برگشت به ابتدای فایل و نوشتن آرایه با اطلاعات فایل‌ها
    out.seekp(0);
    out.write(reinterpret_cast<char*>(fileTable), sizeof(fileTable));

    std::cout << "ساخت ramfs.bin با موفقیت انجام شد.\n";
    return 0;
}