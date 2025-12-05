#ifndef VMM_H
#define VMM_H

#include <stdint.h>

typedef union {
    uint32_t value;

    struct {
        uint32_t present           : 1;
        uint32_t rw                : 1;
        uint32_t user              : 1;
        uint32_t write_through     : 1;
        uint32_t cache_disable     : 1;
        uint32_t accessed          : 1;
        uint32_t reserved          : 1;
        uint32_t page_size         : 1;
        uint32_t global            : 1;
        uint32_t available         : 3;
        uint32_t frame             : 20;
    } __attribute__((packed)) bits;

} PDE32;

typedef union {
    struct {
        uint32_t present    : 1;
        uint32_t rw         : 1;
        uint32_t user       : 1;
        uint32_t pwt        : 1;
        uint32_t pcd        : 1;
        uint32_t accessed   : 1;
        uint32_t dirty      : 1;
        uint32_t pat        : 1;
        uint32_t global     : 1;
        uint32_t available  : 3;
        uint32_t frame      : 20;
    } __attribute__((packed)) bits;
    uint32_t value;
} PTE32;

uint32_t vmm_create_page_directory();
uint32_t vmm_create_page_table();
void load_page_directory(uint32_t pd_address);
void vmm_map(uint32_t pd_address, uint32_t physical_address);
void vmm_unmap(uint32_t pd_address, uint32_t physical_address);
void vmm_init();

#endif