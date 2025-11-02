#include <idt.h>
#include <stdint.h>
#include <binary.h>

/**
 * @brief Interrupt descriptor table entry layout.
 */
typedef struct
{
    uint16_t    BaseLow;          /**< Handler address bits 0-15. */
    uint16_t    SegmentSelector;  /**< Code segment selector for the handler. */
    uint8_t     Reserved;         /**< Unused, must be zero. */
    uint8_t     Flags;            /**< Gate type, privilege, and presence bits. */
    uint16_t    BaseHigh;         /**< Handler address bits 16-31. */
} __attribute__((packed)) IDTEntry;

/**
 * @brief Pseudo-descriptor consumed by the LIDT instruction.
 */
typedef struct 
{
    uint16_t    Limit;            /**< Size of the IDT in bytes minus one. */
    IDTEntry*   Ptr;              /**< Linear address of the first entry. */
} __attribute__((packed)) IDTDescriptor;

/**
 * @brief Global interrupt descriptor table.
 */
IDTEntry g_IDT[256];

/**
 * @brief Descriptor referencing the active IDT.
 */
IDTDescriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT };

/**
 * @brief Assembly helper that loads the IDT descriptor using LIDT.
 *
 * @param idtDescriptor Pointer to the descriptor describing the IDT.
 */
void __attribute__((cdecl)) x86_IDT_Load(IDTDescriptor* idtDescriptor);

/**
 * @brief Configure a single IDT gate.
 *
 * @param interrupt         Vector number to configure.
 * @param base              Pointer to the handler function.
 * @param segmentDescriptor Code segment selector for the handler.
 * @param flags             Gate attributes (type and privilege).
 */
void x86_IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags)
{
    g_IDT[interrupt].BaseLow            = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector    = segmentDescriptor;
    g_IDT[interrupt].Reserved           = 0;
    g_IDT[interrupt].Flags              = flags;
    g_IDT[interrupt].BaseHigh           = ((uint32_t)base >> 16) & 0xFFFF;
}

/**
 * @brief Mark an IDT entry as present.
 *
 * @param interrupt Vector number to enable.
 */
void x86_IDT_EnableGate(int interrupt)
{
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

/**
 * @brief Mark an IDT entry as not present.
 *
 * @param interrupt Vector number to disable.
 */
void x86_IDT_DisableGate(int interrupt)
{
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

/**
 * @brief Load the IDT descriptor into the processor.
 */
void x86_IDT_Initialize()
{
    x86_IDT_Load(&g_IDTDescriptor);
}
