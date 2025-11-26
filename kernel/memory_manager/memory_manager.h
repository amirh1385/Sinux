#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <stdint.h>
#include "../../lib/multiboot_info.h"

/* Page status constants */
#define PAGE_FREE 0x0
#define PAGE_USED 0x2

extern uint32_t memory_size;
extern uint32_t kernel_address;
extern uint8_t *page_map;
extern uint32_t page_map_address;
extern uint32_t total_pages;
extern uint32_t kernel_page_start;

void memory_manager_init(multiboot_info_t* mbi);
uint32_t find_kernel_location(uint32_t size);
uint8_t get_page_status(uint32_t page_num);
void set_page_status(uint32_t page_num, uint8_t status);

#endif