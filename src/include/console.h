#pragma once

#include <stdint.h>
#include <meminit.h>

typedef enum {
    CONSOLE_TEXT,
    CONSOLE_FB
} console_mode_t;

typedef struct Console {
    void (*putchar)(char c);
    void (*puts)(const char* str);
    void (*clear)(void);
    void (*set_cursor)(int x, int y);
} Console;

extern Console* active_console;


void console_init(multiboot_info* mbi);
void putc(const char c);
void write(const char* str);
void clrscr();