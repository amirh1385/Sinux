#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>
#include "inout.h"

#define COM1 0x3F8

static inline void uart_send_char(char c) {
    while (!(inb(COM1 + 5) & 0x20));
    outb(COM1, c);
}

static inline void uart_send_text(const char *str) {
    int i = 0;
    while(str[i]) {
        uart_send_char(str[i]);
        i++;
    }
}

static inline void uart_send_integer(int value) {
    char buffer[12];
    int i = 0;
    bool neg = false;

    if(value < 0) {
        neg = true;
        value = -value;
    }

    do {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    } while(value);

    if(neg) buffer[i++] = '-';

    for(int start = 0, end = i-1; start < end; start++, end--) {
        char tmp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = tmp;
    }

    buffer[i] = '\0';
    uart_send_text(buffer);
}

static inline void uart_send_uint(unsigned int value) {
    char buffer[11];
    int i = 0;

    do {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    } while(value);

    for(int start = 0, end = i-1; start < end; start++, end--) {
        char tmp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = tmp;
    }

    buffer[i] = '\0';
    uart_send_text(buffer);
}

static inline void uart_init() {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

#endif