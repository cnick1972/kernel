#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void kputc(const char c);
void kputs(const char* str);
int  vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int  vsprintf(char *buf, const char *fmt, va_list args);
int  snprintf(char *buf, size_t size, const char *fmt, ...);
int  sprintf(char *buf, const char *fmt, ...);
int  kprintf(const char* fmt, ...);
void kclrscr();
