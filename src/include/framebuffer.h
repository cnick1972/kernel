#pragma once
#include <stdint.h>
#include <multiboot.h>


typedef struct {
    void* address;
    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    uint8_t     bpp;
} Framebuffer;

typedef struct {
    int columns;
    int rows;
} ConsoleSize;

extern Framebuffer fb;

ConsoleSize GetConsoleSize(Framebuffer fb, int char_w, int char_h);
void DrawChar(Framebuffer fb, int x, int y, char c, uint32_t fg, uint32_t bg);
void DrawString(Framebuffer fb, int x, int y, const char* str, uint32_t fg, uint32_t bg);

void InitFramebuffer(multiboot_info* mbi);