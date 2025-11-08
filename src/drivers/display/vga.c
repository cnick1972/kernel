#include <vga.h>
#include <x86.h>
#include <framebuffer.h>

#include <stddef.h>

const unsigned SCREEN_WIDTH = 80;  /**< Number of columns in VGA text mode. */
const unsigned SCREEN_HEIGHT = 25; /**< Number of rows in VGA text mode. */
const uint8_t DEFAULT_COLOR = 0x17; /**< Default attribute byte (foreground/background). */

uint8_t* g_ScreenBuffer = (uint8_t*)0xc00b8000;
int g_ScreenX = 0, g_ScreenY = 0;

/**
 * @brief Write a character cell in the VGA buffer.
 */
void putchr(int x, int y, char c)
{
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

/**
 * @brief Set the colour attribute of a cell.
 */
void putcolor(int x, int y, uint8_t color)
{
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

/**
 * @brief Read the character stored at a VGA cell.
 */
char getchr(int x, int y)
{
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)];
}

/**
 * @brief Read the colour attribute stored at a VGA cell.
 */
uint8_t getcolor(int x, int y)
{
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

/**
 * @brief Update the hardware cursor position.
 */
void setcursor(int x, int y)
{
    int pos = y * SCREEN_WIDTH + x;

    x86_outb(0x3D4, 0x0F);
    x86_outb(0x3D5, (uint8_t)(pos & 0xFF));
    x86_outb(0x3D4, 0x0E);
    x86_outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF)); 
}

/**
 * @brief Clear the VGA text console with the default attribute.
 */
void text_clrscr()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            putchr(x, y, '\0');
            putcolor(x, y, DEFAULT_COLOR);
        }

    g_ScreenX = 0;
    g_ScreenY = 0;
    setcursor(g_ScreenX, g_ScreenY);
}

/**
 * @brief Scroll the text console upward by a number of lines.
 *
 * @param lines Number of lines to scroll.
 */
void scrollback(int lines)
{
    for (int y = lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            putchr(x, y - lines, getchr(x, y));
            putcolor(x, y - lines, getcolor(x, y));
        }

    for (int y = SCREEN_HEIGHT - lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            putchr(x, y, '\0');
            putcolor(x, y, DEFAULT_COLOR);
        }

    g_ScreenY -= lines;
}

/**
 * @brief Render a character on the VGA text console with basic control handling.
 */
void text_putchar(const char c)
{
    switch (c)
    {
        case '\n':
            g_ScreenX = 0;
            g_ScreenY++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (g_ScreenX % 4); i++)
                text_putchar(' ');
            break;

        case '\r':
            g_ScreenX = 0;
            break;

        default:
            putchr(g_ScreenX, g_ScreenY, c);
            g_ScreenX++;
            break;
    }

    if (g_ScreenX >= SCREEN_WIDTH)
    {
        g_ScreenY++;
        g_ScreenX = 0;
    }
    if (g_ScreenY >= SCREEN_HEIGHT)
        scrollback(1);

    setcursor(g_ScreenX, g_ScreenY);
}
