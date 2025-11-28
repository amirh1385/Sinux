#include <stdint.h>

#define ELF_MAGIC 0x464C457F

void load_elf(uint32_t start, uint32_t end){
    if(*(uint32_t*)start == ELF_MAGIC){
        print_string("ELF is valid");
    }else{
        print_string("ELF not valid");
    }
}