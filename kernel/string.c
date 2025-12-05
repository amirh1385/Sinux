#include <stdint.h>

void memcpy(uint32_t source, uint32_t destination, uint32_t size){
    uint8_t *src = (uint8_t*)source;
    uint8_t *dst = (uint8_t*)destination;
    uint32_t remaining = size;

    while(((uintptr_t)src % 8 != 0 || (uintptr_t)dst % 8 != 0) && remaining){
        *dst++ = *src++;
        remaining--;
    }

    uint64_t *src64 = (uint64_t*)src;
    uint64_t *dst64 = (uint64_t*)dst;
    while(remaining >= 8){
        *dst64++ = *src64++;
        remaining -= 8;
    }

    src = (uint8_t*)src64;
    dst = (uint8_t*)dst64;
    while(remaining--){
        *dst++ = *src++;
    }
}

void memset(void *destination, uint8_t value, uint32_t size){
    uint8_t *dst = (uint8_t*)destination;
    uint32_t remaining = size;

    while(((uintptr_t)dst % 8 != 0) && remaining) {
        *dst++ = value;
        remaining--;
    }

    uint64_t val64 = value;
    val64 |= val64 << 8;
    val64 |= val64 << 16;
    val64 |= val64 << 32;

    uint64_t *dst64 = (uint64_t*)dst;
    while(remaining >= 8) {
        *dst64++ = val64;
        remaining -= 8;
    }

    dst = (uint8_t*)dst64;
    while(remaining--) {
        *dst++ = value;
    }
}