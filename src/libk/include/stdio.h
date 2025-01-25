#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void kputc(const char c);
void kputs(const char* str);
int kprintf(const char* fmt, ...);
void kclrscr();