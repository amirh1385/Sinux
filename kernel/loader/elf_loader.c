#include <stdint.h>

#define ELF_MAGIC 0x464C457F

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_header;

void load_elf(uint32_t start, uint32_t end){
    if(*(uint32_t*)start == ELF_MAGIC){
        print_string("ELF is valid");
    }else{
        print_string("ELF not valid");
    }
}