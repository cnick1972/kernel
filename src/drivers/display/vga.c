#include <vga.h>
#include <x86.h>
#include <stddef.h>

const unsigned SCREEN_WIDTH = 80;  /**< Number of columns in VGA text mode. */
const unsigned SCREEN_HEIGHT = 25; /**< Number of rows in VGA text mode. */
const uint8_t DEFAULT_COLOR = 0x17; /**< Default attribute byte (foreground/background). */

static uint8_t* vga_text_buffer = (uint8_t*)0xc00b8000;
static int vga_cursor_x = 0;
static int vga_cursor_y = 0;

/**
 * @brief Write a character cell in the VGA buffer.
 */
static void vga_write_character_cell(int x, int y, char c)
{
    vga_text_buffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

/**
 * @brief Set the colour attribute of a cell.
 */
static void vga_write_color_cell(int x, int y, uint8_t color)
{
    vga_text_buffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

/**
 * @brief Read the character stored at a VGA cell.
 */
static char vga_read_character_cell(int x, int y)
{
    return vga_text_buffer[2 * (y * SCREEN_WIDTH + x)];
}

/**
 * @brief Read the colour attribute stored at a VGA cell.
 */
static uint8_t vga_read_color_cell(int x, int y)
{
    return vga_text_buffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

/**
 * @brief Update the hardware cursor position.
 */
static void vga_update_cursor(int x, int y)
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
void vga_clear(void)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            vga_write_character_cell(x, y, '\0');
            vga_write_color_cell(x, y, DEFAULT_COLOR);
        }

    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_update_cursor(vga_cursor_x, vga_cursor_y);
}

/**
 * @brief Scroll the text console upward by a number of lines.
 *
 * @param lines Number of lines to scroll.
 */
static void vga_scroll(int lines)
{
    for (int y = lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            vga_write_character_cell(x, y - lines, vga_read_character_cell(x, y));
            vga_write_color_cell(x, y - lines, vga_read_color_cell(x, y));
        }

    for (int y = SCREEN_HEIGHT - lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            vga_write_character_cell(x, y, '\0');
            vga_write_color_cell(x, y, DEFAULT_COLOR);
        }

    vga_cursor_y -= lines;
}

/**
 * @brief Render a character on the VGA text console with basic control handling.
 */
void vga_putchar(char c)
{
    switch (c)
    {
        case '\n':
            vga_cursor_x = 0;
            vga_cursor_y++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (vga_cursor_x % 4); i++)
                vga_putchar(' ');
            break;

        case '\r':
            vga_cursor_x = 0;
            break;

        default:
            vga_write_character_cell(vga_cursor_x, vga_cursor_y, c);
            vga_cursor_x++;
            break;
    }

    if (vga_cursor_x >= SCREEN_WIDTH)
    {
        vga_cursor_y++;
        vga_cursor_x = 0;
    }
    if (vga_cursor_y >= SCREEN_HEIGHT)
        vga_scroll(1);

    vga_update_cursor(vga_cursor_x, vga_cursor_y);
}
