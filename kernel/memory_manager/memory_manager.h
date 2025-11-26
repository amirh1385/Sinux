#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <stdint.h>
#include "../../lib/multiboot_info.h"

extern uint32_t memory_size;

void memory_manager_init(multiboot_info_t* mbi);

#endif