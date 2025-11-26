#include "../../lib/multiboot_info.h"
#include <stdint.h>
#include <stddef.h>

/* Paging constants */
#define PAGE_SIZE 4096
#define KERNEL_PAGES 512  /* Number of pages required for kernel */
#define PAGE_DIR_ENTRIES 1024  /* Number of entries in a page directory */
#define PAGE_TABLE_ENTRIES 1024  /* Number of entries in a page table */
#define MAX_PAGE_DIRECTORIES 32  /* Maximum number of page directories */

/* Page status constants */
#define PAGE_FREE 0x0
#define PAGE_USED 0x2

/* Page Directory structure for tracking PD metadata */
typedef struct {
    uint32_t id;  /* Unique identifier for this page directory */
    uint32_t pd_address;  /* Physical address of the page directory */
    uint32_t last_pt_address;  /* Physical address of the last page table */
    uint32_t last_pt_index;  /* Index of the last page table in this directory */
    uint32_t last_entry_index;  /* Last used entry index in the last page table */
    uint32_t used_tables;  /* Number of page tables used in this directory */
} page_directory_t;

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

/* Page directory management */
page_directory_t page_directories[MAX_PAGE_DIRECTORIES];
uint32_t pd_count = 0;  /* Number of active page directories */

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

/**
 * create_page_directory - Create a new page directory and register it
 * @id: Unique identifier for this page directory
 * @pd_address: Physical address where to place the page directory
 * 
 * Returns: Pointer to the new page directory structure, or NULL if limit reached
 */
page_directory_t* create_page_directory(uint32_t id, uint32_t pd_address){
    if(pd_count >= MAX_PAGE_DIRECTORIES){
        return NULL;  /* Maximum page directories reached */
    }
    
    page_directory_t *pd = &page_directories[pd_count];
    pd->id = id;
    pd->pd_address = pd_address;
    pd->last_pt_address = 0;
    pd->last_pt_index = 0;
    pd->last_entry_index = 0;
    pd->used_tables = 0;
    
    pd_count++;
    return pd;
}

/**
 * get_page_directory - Get a page directory by ID
 * @id: Unique identifier of the page directory
 * 
 * Returns: Pointer to the page directory structure, or NULL if not found
 */
page_directory_t* get_page_directory(uint32_t id){
    for(uint32_t i = 0; i < pd_count; i++){
        if(page_directories[i].id == id){
            return &page_directories[i];
        }
    }
    return NULL;
}

/**
 * update_page_directory_state - Update the state of a page directory
 * @pd: Pointer to the page directory
 * @pt_address: Physical address of the current page table
 * @pt_index: Index of the current page table in the directory
 * @entry_index: Index of the last used entry in the page table
 */
void update_page_directory_state(page_directory_t *pd, uint32_t pt_address, 
                                 uint32_t pt_index, uint32_t entry_index){
    if(pd == NULL){
        return;
    }
    
    pd->last_pt_address = pt_address;
    pd->last_pt_index = pt_index;
    pd->last_entry_index = entry_index;
    
    /* Update used_tables if this is a new table */
    if(pt_index >= pd->used_tables){
        pd->used_tables = pt_index + 1;
    }
}

/**
 * get_next_page_table_position - Get the next available position for a page table entry
 * @pd: Pointer to the page directory
 * @out_pt_index: Pointer to store the page table index
 * @out_entry_index: Pointer to store the entry index within the page table
 * 
 * Returns: 1 if position available, 0 if page directory is full
 */
uint32_t get_next_page_table_position(page_directory_t *pd, 
                                       uint32_t *out_pt_index, 
                                       uint32_t *out_entry_index){
    if(pd == NULL){
        return 0;
    }
    
    /* If page table is not full, use next entry in current table */
    if(pd->last_entry_index < PAGE_TABLE_ENTRIES - 1){
        *out_pt_index = pd->last_pt_index;
        *out_entry_index = pd->last_entry_index + 1;
        return 1;
    }
    
    /* If current table is full but more tables available */
    if(pd->last_pt_index < PAGE_DIR_ENTRIES - 1){
        *out_pt_index = pd->last_pt_index + 1;
        *out_entry_index = 0;
        return 1;
    }
    
    /* Page directory is full */
    return 0;
}

/**
 * allocate_free_page - Find and allocate a free page from the page map
 * 
 * Returns: Page number of the allocated page, or 0xFFFFFFFF if no free pages
 */
uint32_t allocate_free_page(void){
    for(uint32_t i = 0; i < total_pages; i++){
        if(page_map[i] == PAGE_FREE){
            return i;
        }
    }
    
    return 0xFFFFFFFF;  /* No free pages found */
}

/**
 * allocate_page_kernel - Allocate a page for kernel use and mark it as used (0x2)
 * 
 * Returns: Physical address of the allocated page, or 0 if allocation fails
 */
uint32_t allocate_page_kernel(void){
    uint32_t page_num = allocate_free_page();
    
    if(page_num == 0xFFFFFFFF){
        return 0;  /* No free pages available */
    }
    
    /* Mark page as used by kernel (0x2) */
    page_map[page_num] = PAGE_USED;
    
    /* Return physical address of the allocated page */
    return page_num * PAGE_SIZE;
}

/**
 * allocate_page - Allocate a page for general use and mark it as used (0x1)
 * 
 * Returns: Physical address of the allocated page, or 0 if allocation fails
 */
uint32_t allocate_page(void){
    uint32_t page_num = allocate_free_page();
    
    if(page_num == 0xFFFFFFFF){
        return 0;  /* No free pages available */
    }
    
    /* Mark page as used (0x1) */
    page_map[page_num] = 0x1;
    
    /* Return physical address of the allocated page */
    return page_num * PAGE_SIZE;
}