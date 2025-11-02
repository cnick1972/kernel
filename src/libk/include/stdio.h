#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Output a character to the active console.
 *
 * @param c Character to write.
 */
void kputc(const char c);

/**
 * @brief Output a null-terminated string to the active console.
 *
 * @param str String to write.
 */
void kputs(const char* str);

/**
 * @brief Format a string into a buffer with explicit size bounds.
 *
 * @param buf   Destination buffer.
 * @param size  Size of the destination buffer in bytes.
 * @param fmt   Format string.
 * @param args  Variadic argument list.
 * @return Number of characters that would have been written (excluding null terminator).
 */
int  vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/**
 * @brief Format a string into a buffer (unbounded).
 *
 * @param buf Destination buffer.
 * @param fmt Format string.
 * @param args Variadic argument list.
 * @return Number of characters written (excluding null terminator).
 */
int  vsprintf(char *buf, const char *fmt, va_list args);

/**
 * @brief Format a string into a buffer with explicit size bounds.
 *
 * @param buf  Destination buffer.
 * @param size Size of the destination buffer in bytes.
 * @param fmt  Format string.
 * @param ...  Additional variadic arguments.
 * @return Number of characters that would have been written (excluding null terminator).
 */
int  snprintf(char *buf, size_t size, const char *fmt, ...);

/**
 * @brief Format a string into a buffer (unbounded variant of snprintf).
 *
 * @param buf Destination buffer.
 * @param fmt Format string.
 * @param ... Additional variadic arguments.
 * @return Number of characters written (excluding null terminator).
 */
int  sprintf(char *buf, const char *fmt, ...);

/**
 * @brief Printf-style formatted output to the active console.
 *
 * @param fmt Format string.
 * @param ... Additional variadic arguments.
 * @return Number of characters written (excluding null terminator).
 */
int  kprintf(const char* fmt, ...);

/**
 * @brief Clear the active console screen.
 */
void kclrscr();
