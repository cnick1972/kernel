/**
 * @file include/console.h
 * @brief Console interface definitions and structures.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

#include <stdint.h>
#include <meminit.h>

/**
 * @brief Supported console backends.
 */
typedef enum {
    CONSOLE_TEXT, /**< Legacy VGA text console. */
    CONSOLE_FB    /**< Linear framebuffer console. */
} console_mode_t;

/**
 * @brief Function table implemented by concrete console backends.
 */
typedef struct console_backend {
    void (*putchar)(char c);               /**< Emit a single character. */
    void (*puts)(const char* str);         /**< Emit a string (optional). */
    void (*clear)(void);                   /**< Clear the display. */
    void (*set_cursor)(int x, int y);      /**< Move the cursor (optional). */
} console_backend_t;

/** @brief Pointer to the currently active console backend. */
extern console_backend_t* g_active_console;

/**
 * @brief Initialize the console subsystem using Multiboot framebuffer info.
 *
 * @param mbi Pointer to the Multiboot information structure.
 */
void console_init(multiboot_info* mbi);

/**
 * @brief Emit a single character via the active console.
 *
 * @param c Character to output.
 */
void console_putc(char c);

/**
 * @brief Emit a null-terminated string via the active console.
 *
 * @param str String to output.
 */
void console_write(const char* str);

/**
 * @brief Clear the active console display.
 */
void console_clear(void);
