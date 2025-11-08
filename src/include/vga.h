#include <stdint.h>

/**
 * @brief PSF1 font header structure.
 */
typedef struct {
    uint16_t magic;     /**< Magic value, e.g. 0x0436. */
    uint8_t mode;       /**< Bitfield: bit0 indicates 512 glyphs if set. */
    uint8_t charsize;   /**< Bytes per glyph. */
} __attribute__((packed)) psf1_header_t;

/**
 * @brief PSF1 font bundle containing header and glyph data pointer.
 */
typedef struct {
    psf1_header_t* header; /**< Pointer to the PSF header. */
    uint8_t* glyphs;       /**< Pointer to glyph bitmap data. */
} psf1_font_t;

/**
 * @brief Render a character to the VGA text console.
 *
 * @param c Character to render.
 */
void text_putchar(const char c);

/**
 * @brief Clear the VGA text console.
 */
void text_clrscr();
