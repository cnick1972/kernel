#pragma once
#include <stdint.h>
#include <multiboot.h>

/**
 * @brief Runtime description of the linear framebuffer.
 */
typedef struct framebuffer {
    void*    address; /**< Virtual address of the framebuffer base. */
    uint32_t width;   /**< Width in pixels. */
    uint32_t height;  /**< Height in pixels. */
    uint32_t pitch;   /**< Bytes between successive scanlines. */
    uint8_t  bpp;     /**< Bits per pixel. */
} framebuffer_t;

extern framebuffer_t g_framebuffer;

/**
 * @brief Initialize the framebuffer console and map video memory.
 *
 * @param mbi Multiboot information block supplied by the bootloader.
 */
void framebuffer_init(multiboot_info* mbi);

/**
 * @brief Clear the framebuffer using the default background colour.
 */
void framebuffer_clear(void);

/**
 * @brief Render a character at the current framebuffer cursor position.
 *
 * @param c Character to draw.
 */
void framebuffer_putchar(char c);
