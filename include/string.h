#ifndef STRING_H
#define STRING_H

#include <stdint.h>

void memcpy(uint32_t source, uint32_t destination, uint32_t size);
void memset(void *destination, uint8_t value, uint32_t size);

#endif