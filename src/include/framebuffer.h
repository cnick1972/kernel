#pragma once
#include <stdint.h>
#include <multiboot.h>

/**
 * @brief Runtime description of the linear framebuffer.
 */
typedef struct {
    void*    address; /**< Virtual address of the framebuffer base. */
    uint32_t width;   /**< Width in pixels. */
    uint32_t height;  /**< Height in pixels. */
    uint32_t pitch;   /**< Bytes between successive scanlines. */
    uint8_t  bpp;     /**< Bits per pixel. */
} Framebuffer;

/**
 * @brief Helper describing console dimensions in character cells.
 */
typedef struct {
    int columns; /**< Number of columns. */
    int rows;    /**< Number of rows. */
} ConsoleSize;

extern Framebuffer fb;

/**
 * @brief Compute console dimensions for a framebuffer and glyph size.
 *
 * @param fb      Framebuffer under consideration.
 * @param char_w  Character width in pixels.
 * @param char_h  Character height in pixels.
 * @return Console dimensions measured in characters.
 */
ConsoleSize GetConsoleSize(Framebuffer fb, int char_w, int char_h);

/**
 * @brief Initialize the framebuffer console and map video memory.
 *
 * @param mbi Multiboot information block supplied by the bootloader.
 */
void InitFramebuffer(multiboot_info* mbi);

/**
 * @brief Clear the framebuffer using the default background colour.
 */
void fb_clrscr();

/**
 * @brief Render a character at the current framebuffer cursor position.
 *
 * @param c Character to draw.
 */
void fb_putchar(char c);
