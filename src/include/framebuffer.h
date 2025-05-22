#pragma once
#include <stdint.h>
#include <multiboot.h>




/***
 * @brief Represents the linear framebuffer
 * 
 * This is the linear framebuffer, and the require attributes
 */
typedef struct {
    /**
     * @brief The virtual address of the framebuffer.
     * 
     */
    void* address;
    /**
     * @brief The width of the screen in pixels
     * 
     */
    uint32_t    width;
    /**
     * @brief The height of the screen in pixels
     */
    uint32_t    height;
    /**
     * @brief The pitch of the screen, ie how many bytes to the same location on the next line
     * 
     */
    uint32_t    pitch;
    /**
     * @brief The bits per pixel
     * 
     */
    uint8_t     bpp;
} Framebuffer;

typedef struct {
    int columns;
    int rows;
} ConsoleSize;

extern Framebuffer fb;

ConsoleSize GetConsoleSize(Framebuffer fb, int char_w, int char_h);

void InitFramebuffer(multiboot_info* mbi);
void fb_clrscr();
void fb_putchar(char c);