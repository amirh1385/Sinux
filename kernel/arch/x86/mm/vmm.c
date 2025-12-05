#include <arch/x86/mm/pmm.h>
#include <arch/x86/mm/vmm.h>
#include <log.h>
#include <stdint.h>

uint32_t vmm_create_page_directory(){
    uint32_t pd_address = pmm_reserve_kernel_page(1);

    if(pd_address == 0xFFFFFFFF){
        return 0xFFFFFFFF;
    }

    for(int i = pd_address; i < 512; i+=8){
        *(uint64_t*)i = 0;
    }

    return pd_address;
}

uint32_t vmm_create_page_table(){
    uint32_t pt_address = pmm_reserve_kernel_page(1);

    if(pt_address == 0xFFFFFFFF){
        return 0xFFFFFFFF;
    }

    for(int i = pt_address; i < 512; i+=8){
        *(uint64_t*)i = 0;
    }

    return pt_address;
}

void vmm_map(uint32_t pd_address, uint32_t physical_address){
    PDE32* pd = (PDE32*)(pd_address + (physical_address >> 22) * 4);
    if(!pd->bits.present){
        uint32_t pt_address = vmm_create_page_table();
        pd->bits.frame = pt_address >> 12;
        pd->bits.rw = 1;
        pd->bits.user = 1;
        pd->bits.present = 1;
    }

    uint32_t pt_address = pd->bits.frame << 12;
    uint32_t pt_index = (physical_address >> 12) & 0x3FF;

    PTE32* pt = (PTE32*)pt_address;
    pt[pt_index].bits.frame = physical_address >> 12;
    pt[pt_index].bits.rw = 1;
    pt[pt_index].bits.user = 1;
    pt[pt_index].bits.present = 1;
}

void vmm_unmap(uint32_t pd_address, uint32_t physical_address){
    PDE32* pd = (PDE32*)(pd_address + (physical_address >> 22) * 4);

    if (!pd->bits.present) {
        return;
    }

    uint32_t pt_address = pd->bits.frame << 12;

    uint32_t pt_index = (physical_address >> 12) & 0x3FF;
    PTE32* pt = (PTE32*)pt_address;

    if (pt[pt_index].bits.present) {
        pt[pt_index].bits.present = 0;
        pt[pt_index].bits.rw = 0;
        pt[pt_index].bits.user = 0;
        pt[pt_index].bits.frame = 0;
    }

    bool empty = true;
    for (int i = 0; i < 1024; i++) {
        if (pt[i].bits.present) {
            empty = false;
            break;
        }
    }

    if (empty) {
        pd->bits.present = 0;
        pd->bits.frame = 0;
    }
}

void load_page_directory(uint32_t pd_address) {
    __asm__ volatile(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"    
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        :
        : "r"(pd_address)
        : "eax", "memory"
    );
}

void vmm_init(){
    uint32_t pd_address = vmm_create_page_directory();

    for(uint32_t i = 0; i < pmm_pages_count; i++){
        vmm_map(pd_address, i*4096);
    }

    if(pd_address == 0xFFFFFFFF){
        uart_send_text("page directory creation failed");
        while(1){}
    }

    load_page_directory(pd_address);
}