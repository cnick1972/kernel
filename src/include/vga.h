#include <stdint.h>

typedef struct {
    uint16_t magic;     // 0x0436
    uint8_t mode;       // bit 0: 512 glyphs if set, 256 if clear
    uint8_t charsize;   // bytes per character
} __attribute__((packed)) psf1_header_t;

typedef struct {
    psf1_header_t* header;
    uint8_t* glyphs;
} psf1_font_t;

void text_putchar(const char c);
void text_clrscr();