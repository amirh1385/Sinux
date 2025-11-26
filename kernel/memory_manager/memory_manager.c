#include "../../lib/multiboot_info.h"
#include <stdint.h>
#include <stddef.h>

/* Paging constants */
#define PAGE_SIZE 4096
#define KERNEL_PAGES 512  /* Number of pages required for kernel */

/* Page status constants */
#define PAGE_FREE 0x0
#define PAGE_USED 0x2

/* Forward declarations */
uint32_t find_kernel_location(uint32_t size);

/* Global pointer to the Multiboot memory map and entry count */
multiboot_memory_map_t *memory_map;
size_t memory_map_entries;

uint32_t memory_size;
uint32_t kernel_address;  /* Physical address where kernel will be placed */
uint8_t *page_map;  /* Pointer to page allocation map */
uint32_t page_map_address;  /* Address where page allocation map starts */
uint32_t total_pages;  /* Total number of pages in system */
uint32_t kernel_page_start;  /* Starting page number for kernel */

void memory_manager_init(multiboot_info_t* mbi){
    memory_map = (multiboot_memory_map_t*)(uintptr_t)(mbi->mmap_addr);
    memory_map_entries = mbi->mmap_length;

    /* Calculate total system RAM size and find maximum memory address */
    memory_size = 0;
    uint32_t max_addr = 0;
    multiboot_memory_map_t *entry = memory_map;
    uint32_t entries_count = mbi->mmap_length / sizeof(multiboot_memory_map_t);
    
    for(uint32_t i = 0; i < entries_count; i++){
        uint32_t end_addr = (uint32_t)(entry->addr + entry->len);
        if(end_addr > max_addr){
            max_addr = end_addr;
        }
        if(entry->type == 1){ /* type 1 = available memory */
            memory_size += entry->len;
        }
        entry++;
    }

    /* Find suitable location in RAM for kernel pages */
    uint32_t kernel_size = KERNEL_PAGES * PAGE_SIZE;
    kernel_address = find_kernel_location(kernel_size);
    
    if(kernel_address == 0){
        return;  /* No suitable location found */
    }

    /* Page map will be placed right after kernel pages */
    kernel_page_start = kernel_address / PAGE_SIZE;
    page_map_address = kernel_address + kernel_size;
    
    /* Calculate total pages and initialize page allocation map */
    total_pages = (max_addr + PAGE_SIZE - 1) / PAGE_SIZE;
    page_map = (uint8_t*)page_map_address;
    
    /* Initialize all pages as used (0x2) for unavailable memory */
    for(uint32_t i = 0; i < total_pages; i++){
        page_map[i] = PAGE_USED;
    }
    
    /* Mark available memory pages as free */
    entry = memory_map;
    for(uint32_t i = 0; i < entries_count; i++){
        if(entry->type == 1){ /* Available memory */
            uint32_t start_page = (uint32_t)(entry->addr / PAGE_SIZE);
            uint32_t end_page = start_page + (entry->len / PAGE_SIZE);
            
            for(uint32_t page = start_page; page < end_page; page++){
                page_map[page] = PAGE_FREE;
            }
        }
        entry++;
    }
    
    /* Mark kernel pages as used */
    for(uint32_t i = 0; i < KERNEL_PAGES; i++){
        page_map[kernel_page_start + i] = PAGE_USED;
    }
}

/**
 * find_kernel_location - Find a suitable location in RAM for kernel
 * @size: Required size in bytes
 * 
 * Returns: Physical address of available memory space, or 0 if not found
 */
uint32_t find_kernel_location(uint32_t size){
    multiboot_memory_map_t *entry = memory_map;
    uint32_t entries_count = memory_map_entries / sizeof(multiboot_memory_map_t);
    
    for(uint32_t i = 0; i < entries_count; i++){
        if(entry->type == 1){ /* Available memory */
            /* Check if this block has enough space */
            if(entry->len >= size){
                return (uint32_t)entry->addr;
            }
        }
        entry++;
    }
    
    return 0;  /* No suitable location found */
}

/**
 * get_page_status - Get the status of a specific page
 * @page_num: Page number to check
 * 
 * Returns: Page status (PAGE_FREE or PAGE_USED)
 */
uint8_t get_page_status(uint32_t page_num){
    if(page_num >= total_pages){
        return PAGE_USED;
    }
    return page_map[page_num];
}

/**
 * set_page_status - Set the status of a specific page
 * @page_num: Page number to set
 * @status: Status value (PAGE_FREE or PAGE_USED)
 */
void set_page_status(uint32_t page_num, uint8_t status){
    if(page_num < total_pages){
        page_map[page_num] = status;
    }
}