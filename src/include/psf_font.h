#pragma once

#include <stdint.h>
#include <stddef.h>


/**
 * @brief PSF1 font header structure.
 */
typedef struct {
    uint16_t magic;   /**< Magic value identifying PSF1 fonts. */
    uint8_t mode;     /**< Mode flags (bit 0 indicates 512 glyphs). */
    uint8_t charsize; /**< Bytes per glyph. */
} __attribute__((packed)) psf1_header_t;

/**
 * @brief Loaded PSF1 font containing header and glyph pointer.
 */
typedef struct {
    psf1_header_t* header; /**< Pointer to PSF header. */
    uint8_t* glyphs;       /**< Pointer to glyph bitmap data. */
} psf1_font_t;

/**
 * @brief Load the built-in PSF font embedded in the kernel image.
 *
 * @return Font structure with header/glyph pointers, or NULL pointers on failure.
 */
psf1_font_t load_psf1_font();
