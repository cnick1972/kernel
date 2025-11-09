#include <framebuffer.h>
#include <meminit.h>
#include <memory.h>
#include <x86.h>
#include <psf_font.h>
#include <stddef.h>
#include <serial.h>

/** @brief Loaded PSF font used for framebuffer text rendering. */
static psf1_font_t framebuffer_font;
/** @brief Number of 4 KiB pages needed for the framebuffer mapping. */
static uint32_t framebuffer_total_pages_required;
/** @brief Global framebuffer descriptor. */
framebuffer_t g_framebuffer = {NULL, 0, 0, 0, 0};

/** @brief Character backing buffer for framebuffer console text. */
static uint8_t* framebuffer_char_buffer = (uint8_t*)0xc1000000;

static uint32_t framebuffer_columns;

static int framebuffer_pixel_x = 0;
static int framebuffer_pixel_y = 0;
static int console_column = 0;
static int console_row = 0;

#define FONT_HEIGHT     16
#define FONT_WIDTH      8

/**
 * @brief Initialize framebuffer mappings and prepare the console renderer.
 */
void framebuffer_init(multiboot_info* mbi)
{
    g_framebuffer.address = (void*)(uint32_t)mbi->framebuffer_addr;
    g_framebuffer.height = mbi->framebuffer_height;
    g_framebuffer.width = mbi->framebuffer_width;
    g_framebuffer.bpp = mbi->framebuffer_bpp;
    g_framebuffer.pitch = mbi->framebuffer_pitch;

    framebuffer_columns = g_framebuffer.width / FONT_WIDTH;

    for(int i = 0; i < 4; i++)
    {
        
        vmm_map_physical_to_virtual((uint8_t*)(uintptr_t)pmm_allocate_page(), framebuffer_char_buffer + i * 4096);
    }

    
    uintptr_t framebuffer_virtual_address = 0xe0000000;


    // currently the framebuffer physical address needs to be mapped to
    // a virtual address space.  Convention for x86 is at address 0xe0000000.

    // First part is to calculate how many 4k pages are required.  This is foung by multiplying the
    // height by width to calculate the number of pixels, the multiplying by the bytes per pixels.
    // bytes per pixels is calculated by dividing bpp by 8, of shift 3 bit right.

    // We should just alocate 4MB pages for video memory

    int bytes_per_pixel = g_framebuffer.bpp >> 3;
    uint32_t total_screen_pixels = g_framebuffer.height * g_framebuffer.width;
    framebuffer_total_pages_required = (total_screen_pixels * bytes_per_pixel) / 4096;
    if((total_screen_pixels * bytes_per_pixel) % 4096)
        framebuffer_total_pages_required++;

    uint8_t four_mb_pages_required = framebuffer_total_pages_required / 1024;
    if(framebuffer_total_pages_required % 1024)
        four_mb_pages_required++;

    vmm_map_4mb_physical_to_virtual((uint8_t*)g_framebuffer.address, (uint8_t*)framebuffer_virtual_address, four_mb_pages_required * 4);
    
    // set the framebuffer address in the framebuffer structure to the virtual address

    g_framebuffer.address = (void*)framebuffer_virtual_address;

    // get the builtin font
    framebuffer_font = psf1_font_load();

    SerialPrintf("Font magic: 0x%04x, Mode: %d, Size: %d\n", framebuffer_font.header->magic, framebuffer_font.header->mode, framebuffer_font.header->charsize);
}

/**
 * @brief Write a pixel to the framebuffer.
 *
 * @param x     X coordinate in pixels.
 * @param y     Y coordinate in pixels.
 * @param color ARGB color value.
 */
void putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    uint32_t* pixel = (uint32_t*)((uint8_t*)g_framebuffer.address + y * g_framebuffer.pitch + x * 4);
    *pixel = color;
}

/**
 * @brief Draw a single glyph at the specified framebuffer position.
 *
 * @param c        Character to draw.
 * @param x        X coordinate in pixels.
 * @param y        Y coordinate in pixels.
 * @param fg_color Foreground color.
 * @param bg_color Background color.
 */
void drawchar(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color)
{
    uint8_t* glyph = framebuffer_font.glyphs +((uint8_t)c * framebuffer_font.header->charsize);

    for(int row = 0; row < framebuffer_font.header->charsize; row++)
    {
        uint8_t bits = glyph[row];

        for(int col = 0; col < FONT_WIDTH; col++)
        {
            uint32_t color = (bits & (0x80 >> col)) ? fg_color : bg_color;
            putpixel(x + col, y + row, color);
        }
    }
}

/**
 * @brief Fill the framebuffer with the default background colour.
 */
void framebuffer_clear(void)
{
    uint32_t* frame = g_framebuffer.address;
    for(uint32_t i = 0; i < g_framebuffer.width * g_framebuffer.height; i++)
        frame[i] = 0x002366;
}


/**
 * @brief Render a character and advance the framebuffer cursor.
 *
 * @param c Character to render.
 */
void framebuffer_putchar(char c)
{
    if(framebuffer_font.header == NULL || framebuffer_font.glyphs == NULL)
    {
        return;
    }

    switch(c)
    {
    case '\n':
        console_column = 0;
        console_row++;
        framebuffer_pixel_x = 0;    
        framebuffer_pixel_y += FONT_HEIGHT;
        break;

    default:
        drawchar(c, framebuffer_pixel_x, framebuffer_pixel_y, 0x00ffff00, 0x002366);
        framebuffer_pixel_x += FONT_WIDTH;
        if(framebuffer_columns != 0)
        {
            framebuffer_char_buffer[console_row * framebuffer_columns + console_column] = c;
        }

        console_column++;
        if(framebuffer_columns != 0 && console_column >= (int)framebuffer_columns)
        {
            console_column = 0;
            console_row++;
            framebuffer_pixel_x = 0;
            framebuffer_pixel_y += FONT_HEIGHT;
        }
        break;
    }
}
