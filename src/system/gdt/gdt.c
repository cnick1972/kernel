#include <gdt.h>
#include <stdint.h>
#include <kerndef.h>

typedef struct gdt_descriptor gdt_descriptor_t;

/**
 * @brief Assembly helper that loads a GDT descriptor and resets segment registers.
 *
 * @param descriptor  Pointer to the GDT descriptor.
 * @param codeSegment Selector to load into CS.
 * @param dataSegment Selector to load into the remaining segment registers.
 */
void KERNEL_CDECL gdt_load(gdt_descriptor_t* descriptor, uint16_t code_segment, uint16_t data_segment);

/**
 * @brief Layout of an individual GDT entry (segment descriptor).
 */
typedef struct
{
    uint16_t LimitLow;                  /**< Segment limit bits 0-15. */
    uint16_t BaseLow;                   /**< Base address bits 0-15. */
    uint8_t  BaseMiddle;                /**< Base address bits 16-23. */
    uint8_t  Access;                    /**< Access rights and type field. */
    uint8_t  FlagsLimitHi;              /**< High limit bits combined with flags. */
    uint8_t  BaseHigh;                  /**< Base address bits 24-31. */
} __attribute__((packed)) gdt_entry_t;

/**
 * @brief Pseudo-descriptor passed to the LGDT instruction.
 */
struct gdt_descriptor
{
    uint16_t Limit;                     /**< Size of the GDT in bytes minus one. */
    gdt_entry_t* Ptr;                   /**< Linear address of the first GDT entry. */
} __attribute__((packed));

/**
 * @brief Access-byte helpers for GDT entries.
 */
typedef enum
{
    GDT_ACCESS_CODE_READABLE                = 0x02,
    GDT_ACCESS_DATA_WRITEABLE               = 0x02,

    GDT_ACCESS_CODE_CONFORMING              = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

    GDT_ACCESS_DATA_SEGMENT                 = 0x10,
    GDT_ACCESS_CODE_SEGMENT                 = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS               = 0x00,

    GDT_ACCESS_RING0                        = 0x00,
    GDT_ACCESS_RING1                        = 0x20,
    GDT_ACCESS_RING2                        = 0x40,
    GDT_ACCESS_RING3                        = 0x60,

    GDT_ACCESS_PRESENT                      = 0x80,

} GDT_ACCESS;

/**
 * @brief Flag helpers for GDT entries (upper four bits of the flags/limit byte).
 */
typedef enum 
{
    GDT_FLAG_64BIT                          = 0x20,
    GDT_FLAG_32BIT                          = 0x40,
    GDT_FLAG_16BIT                          = 0x00,

    GDT_FLAG_GRANULARITY_1B                 = 0x00,
    GDT_FLAG_GRANULARITY_4K                 = 0x80,
} GDT_FLAGS;

// Helper macros
#define GDT_LIMIT_LOW(limit)                (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) {                     \
    GDT_LIMIT_LOW(limit),                                           \
    GDT_BASE_LOW(base),                                             \
    GDT_BASE_MIDDLE(base),                                          \
    access,                                                         \
    GDT_FLAGS_LIMIT_HI(limit, flags),                               \
    GDT_BASE_HIGH(base)                                             \
}

/**
 * @brief Global descriptor table containing kernel code/data segments.
 */
static gdt_entry_t gdt_entries[] = {
    // NULL descriptor
    GDT_ENTRY(0, 0, 0, 0),

    // Kernel 32-bit code segment
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // Kernel 32-bit data segment
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

};

/**
 * @brief Descriptor referencing the active GDT.
 */
static gdt_descriptor_t gdt_descriptor = { sizeof(gdt_entries) - 1, gdt_entries};

/**
 * @brief Populate and load the kernel global descriptor table.
 */
void gdt_init(void)
{
    gdt_load(&gdt_descriptor, GDT_SELECTOR_CODE, GDT_SELECTOR_DATA);
}
