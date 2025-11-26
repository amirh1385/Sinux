#include "../../lib/multiboot_info.h"
#include <stdint.h>
#include <stddef.h>

/* Global pointer to the Multiboot memory map and entry count */
multiboot_memory_map_t *memory_map;
size_t memory_map_entries;

uint32_t memory_size;

void memory_manager_init(multiboot_info_t* mbi){
    memory_map = (multiboot_memory_map_t*)(uintptr_t)(mbi->mmap_addr);
    memory_map_entries = mbi->mmap_length;

    /* Calculate total system RAM size */
    memory_size = 0;
    multiboot_memory_map_t *entry = memory_map;
    uint32_t entries_count = mbi->mmap_length / sizeof(multiboot_memory_map_t);
    
    for(uint32_t i = 0; i < entries_count; i++){
        if(entry->type == 1){ /* type 1 = available memory */
            memory_size += entry->len;
        }
        entry++;
    }
}