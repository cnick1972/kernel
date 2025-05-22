#pragma once

#include <stdint.h>
#include <stddef.h>


typedef struct {
    uint16_t magic;
    uint8_t mode;
    uint8_t charsize;
} __attribute__((packed)) psf1_header_t;

typedef struct {
    psf1_header_t* header;
    uint8_t* glyphs;
} psf1_font_t;

psf1_font_t load_psf1_font();