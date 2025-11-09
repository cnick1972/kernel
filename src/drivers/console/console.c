#include <console.h>
#include <vga.h>
#include <framebuffer.h>
#include <serial.h>
#include <stddef.h>

enum framebuffer_type {
    FRAMEBUFFER_TYPE_INDEXED   = 0, /**< Multiboot framebuffer type: indexed palette. */
    FRAMEBUFFER_TYPE_RGB       = 1, /**< Multiboot framebuffer type: RGB. */
    FRAMEBUFFER_TYPE_TEXT      = 2, /**< Multiboot framebuffer type: VGA text mode. */
};

/** @brief Console implementation backed by VGA text mode. */
static console_backend_t text_console = {
    .putchar = vga_putchar,
    .puts = NULL,
    .clear = vga_clear,
    .set_cursor = NULL
};

/** @brief Console implementation backed by the linear framebuffer. */
static console_backend_t framebuffer_console = {
    .putchar = framebuffer_putchar,
    .puts = NULL,
    .clear = framebuffer_clear,
    .set_cursor = NULL
};

console_backend_t* g_active_console = NULL;

/**
 * @brief Initialize the console backend based on the reported framebuffer type.
 */
void console_init(multiboot_info* mbi)
{
    switch(mbi->framebuffer_type)
    {
    case FRAMEBUFFER_TYPE_INDEXED:
        break;

    case FRAMEBUFFER_TYPE_RGB:
        framebuffer_init(mbi);

        uint32_t* frame = g_framebuffer.address;
        for(uint32_t i = 0; i < (g_framebuffer.width * g_framebuffer.height); i++)
        {
            frame[i] = 0x002366;
        }

        g_active_console = &framebuffer_console;
        break;

    case FRAMEBUFFER_TYPE_TEXT:
        SerialPrintf("In FB_TEXT\n");
        g_active_console = &text_console;
        SerialPrintf("Active 0x%08x, text 0x%08x\n", g_active_console, &text_console);

        break;

    default:
        break;
    }

    if(g_active_console == NULL)
    {
        g_active_console = &text_console;
    }
}

/**
 * @brief Emit a character through the active console backend.
 */
void console_putc(char c)
{
    if(g_active_console && g_active_console->putchar != NULL)
        g_active_console->putchar(c);
}

/**
 * @brief Emit a string through the active console backend.
 */
void console_write(const char* str)
{
    if(str == NULL)
    {
        return;
    }

    while(*str)
    {
        console_putc(*str++);
    }
}

/**
 * @brief Clear the active console display.
 */
void console_clear(void)
{
    if(g_active_console && g_active_console->clear != NULL)
        g_active_console->clear();
}
