#include <x86.h>
#include <stdarg.h>
#include <stdio.h>

#define COM1 0x3f8

/**
 * @brief Initialize COM1 (0x3F8) for 115200 8-N-1 operation.
 */
void serial_init(void)
{
    x86_outb(COM1 +1, 0x00);
    x86_outb(COM1 +3, 0x80);
    x86_outb(COM1 +0, 0x03);
    x86_outb(COM1 +1, 0x00);
    x86_outb(COM1 +3, 0x03);
    x86_outb(COM1 +2, 0xc7);
    x86_outb(COM1 +4, 0x0b);      
}

static uint8_t serial_is_transmit_empty(void)
{
    return x86_inb(COM1 + 5) & 0x20;
}

/**
 * @brief Blocking write of a single character to COM1.
 */
void serial_write_char(char c)
{
    while(!serial_is_transmit_empty());

    x86_outb(COM1, c);
}

void serial_write_string(const char* str) {
    while (*str) {
        if (*str == '\n') serial_write_char('\r'); // For proper line endings
        serial_write_char(*str++);
    }
}

/**
 * @brief Printf-style formatted string over the serial console.
 */
void serial_printf(const char* fmt, ...) 
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (written < 0)
        return;

    if ((size_t)written >= sizeof(buf))
        written = sizeof(buf) - 1;

    buf[written] = '\0';
    serial_write_string(buf);
}
