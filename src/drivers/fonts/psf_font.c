#include <psf_font.h>

#define PSF1_MAGIC 0x0436

extern uint8_t _binary_src_fonts_Lat15_Terminus16_psf_start[];
extern uint8_t _binary_src_fonts_Lat15_Terminus16_psf_end[];

psf1_font_t load_psf1_font()
{
    psf1_font_t font;
    font.header = (psf1_header_t*)_binary_src_fonts_Lat15_Terminus16_psf_start;

    if(font.header->magic != PSF1_MAGIC)
    {
        font.header = NULL;
        font.glyphs = NULL;
        return font;
    }
    font.glyphs = _binary_src_fonts_Lat15_Terminus16_psf_start + sizeof(psf1_header_t);
    return font;
}