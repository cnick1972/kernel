#include <console.h>
#include <vga.h>
#include <framebuffer.h>
#include <serial.h>

#define FB_INDEXED_COLOUR   0 /**< Multiboot framebuffer type: indexed palette. */
#define FB_RGB_COLOUR       1 /**< Multiboot framebuffer type: RGB. */
#define FB_TEXT             2 /**< Multiboot framebuffer type: VGA text mode. */

/** @brief Console implementation backed by VGA text mode. */
Console text_console = {
    .putchar = text_putchar,
    .puts = NULL,
    .clear = text_clrscr,
    .set_cursor = NULL
};

/** @brief Console implementation backed by the linear framebuffer. */
Console fb_console = {fb_putchar, NULL, fb_clrscr, NULL};

Console* active_console = NULL;

/**
 * @brief Initialize the console backend based on the reported framebuffer type.
 */
void console_init(multiboot_info* mbi)
{
    switch(mbi->framebuffer_type)
    {
    case FB_INDEXED_COLOUR:
        break;

    case FB_RGB_COLOUR:
        InitFramebuffer(mbi);

        uint32_t* frame = fb.address;
        for(uint32_t i = 0; i < (fb.width * fb.height); i++)
        {
            frame[i] = 0x002366;
        }

        active_console = &fb_console;
        break;

    case FB_TEXT:
        SerialPrintf("In FB_TEXT\n");
        active_console = &text_console;

        SerialPrintf("Active 0x%08x, text 0x%08x\n", active_console, &text_console);

        break;

    default:
        break;
    }
}

/**
 * @brief Emit a character through the active console backend.
 */
void putc(const char c)
{
    if(active_console->putchar != NULL)
        active_console->putchar(c);
}

/**
 * @brief Emit a string through the active console backend.
 */
void write(const char* str)
{

}

/**
 * @brief Clear the active console display.
 */
void clrscr()
{
    if(active_console->clear != NULL)
        active_console->clear();
}
