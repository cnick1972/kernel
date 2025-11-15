/**
 * @file system/gdt/gdt.c
 * @brief Global Descriptor Table construction.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

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

    // User 32-bit code segment
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // User 32-bit data segment
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // TSS placeholder (patched at runtime)
    GDT_ENTRY(0,
              0,
              0,
              0),
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

void gdt_set_entry_raw(int index,
                       uint32_t base,
                       uint32_t limit,
                       uint8_t access,
                       uint8_t flags)
{
    gdt_entries[index].LimitLow = GDT_LIMIT_LOW(limit);
    gdt_entries[index].BaseLow = GDT_BASE_LOW(base);
    gdt_entries[index].BaseMiddle = GDT_BASE_MIDDLE(base);
    gdt_entries[index].Access = access;
    gdt_entries[index].FlagsLimitHi = GDT_FLAGS_LIMIT_HI(limit, flags);
    gdt_entries[index].BaseHigh = GDT_BASE_HIGH(base);
}
