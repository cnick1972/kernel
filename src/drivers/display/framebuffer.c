#include <framebuffer.h>
#include <meminit.h>
#include <memory.h>
#include <x86.h>
#include <psf_font.h>
#include <stddef.h>
#include <serial.h>

psf1_font_t font;
uint32_t TotalPagesRequired;
Framebuffer fb = {NULL, 0, 0, 0, 0};

uint8_t* CharBuffer = (uint8_t*)0xc1000000;

uint8_t columns;
uint8_t rows;

int fb_ScreenX = 0, fb_ScreenY = 0;
int c_ScreenX = 0, c_screenY = 0;



#define FONT_HEIGHT     16
#define FONT_WIDTH      8

void InitFramebuffer(multiboot_info* mbi)
{
    fb.address = (void*)(uint32_t)mbi->framebuffer_addr;
    fb.height = mbi->framebuffer_height;
    fb.width = mbi->framebuffer_width;
    fb.bpp = mbi->framebuffer_bpp;
    fb.pitch = mbi->framebuffer_pitch;

    columns = fb.width / FONT_WIDTH;
    rows = fb.height / FONT_HEIGHT;

    for(int i = 0; i < 4; i++)
    {
        
        MapPhysicalToVirtual((void*)allocate_physical_page(), CharBuffer + i * 4096);
    }

    
    uint32_t FrameBufferVirtualAddress = 0xe0000000;


    // currently the fb.address is a physical address, it needs to be mapped to
    // a virtual address space.  Convention for x86 is at address 0xe0000000.

    // First part is to calculate how many 4k pages are required.  This is foung by multiplying the
    // height by width to calculate the number of pixels, the multiplying by the bytes per pixels.
    // bytes per pixels is calculated by dividing bpp by 8, of shift 3 bit right.

    // We should just alocate 4MB pages for video memory

    int BytesPerPixel = fb.bpp >> 3;
    uint32_t TotalScreenPixels = fb.height * fb.width;
    TotalPagesRequired = (TotalScreenPixels * BytesPerPixel) / 4096;
    if((TotalScreenPixels * BytesPerPixel) % 4096)
        TotalPagesRequired++;

    uint8_t FourMBPagesRequied = TotalPagesRequired / 1024;
    if(TotalPagesRequired % 1024)
        FourMBPagesRequied++;

    Map4MBPhysicalToVirtual(fb.address, (uint8_t*)FrameBufferVirtualAddress, FourMBPagesRequied * 4);
    
    // set the framebuffer address in the framebuffer structure to the virtual address

    fb.address = (void*)FrameBufferVirtualAddress;

    // get the builtin font
    font = load_psf1_font();

    SerialPrintf("Font magic: 0x%04x, Mode: %d, Size: %d\n", font.header->magic, font.header->mode, font.header->charsize);
}

void putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    uint32_t* pixel = (uint32_t*)((uint8_t*)fb.address + y * fb.pitch + x * 4);
    *pixel = color;
}

void drawchar(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color)
{
    uint8_t* glyph = font.glyphs +((uint8_t)c * font.header->charsize);

    for(int row = 0; row < font.header->charsize; row++)
    {
        uint8_t bits = glyph[row];

        for(int col = 0; col < FONT_WIDTH; col++)
        {
            uint32_t color = (bits & (0x80 >> col)) ? fg_color : bg_color;
            putpixel(x + col, y + row, color);
        }
    }
}

void fb_clrscr()
{
    uint32_t* frame = fb.address;
    for(uint32_t i = 0; i < fb.width * fb.height; i++)
        frame[i] = 0x002366;
}


void fb_putchar(char c)
{
    switch(c)
    {
    case '\n':
        c_ScreenX = 0;
        c_screenY++;
        fb_ScreenX = 0;    
        fb_ScreenY += 16;
        break;

    default:
        drawchar(c, fb_ScreenX, fb_ScreenY, 0x00ffff00, 0x002366);
        fb_ScreenX += 8;
        CharBuffer[c_screenY * columns + c_ScreenX] = c;
        if(c_ScreenX++ > columns)
        {
            c_ScreenX = 0;
            c_screenY++;
            fb_ScreenX = 0;
            fb_ScreenY += 16;
        }
        break;
    }
}