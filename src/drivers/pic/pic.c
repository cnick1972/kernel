#include <stdint.h>
#include <pic.h>
#include <x86.h>

#define PIC1_COMMAND_PORT           0x20 /**< Master PIC command port. */
#define PIC1_DATA_PORT              0x21 /**< Master PIC data port. */
#define PIC2_COMMAND_PORT           0xA0 /**< Slave PIC command port. */
#define PIC2_DATA_PORT              0xA1 /**< Slave PIC data port. */

/** @brief Bit definitions for PIC initialization control word 1. */
enum {
    PIC_ICW1_ICW4               = 0b00000001,
    PIC_ICW1_SINGLE             = 0b00000010,
    PIC_ICW1_INTERVAL4          = 0b00000100,
    PIC_ICW1_LEVEL              = 0b00001000,
    PIC_ICW1_INITIALIZE         = 0b00010000,
} PIC_ICW1;

/** @brief Bit definitions for PIC initialization control word 4. */
enum {
    PIC_ICW4_8086               = 0b00000001,
    PIC_ICW4_AUTO_EOI           = 0b00000010,
    PIC_ICW4_BUFFER_MASTER      = 0b00000100,
    PIC_ICW4_BUFFER_SLAVE       = 0b00000000,
    PIC_ICW4_BUFFERED           = 0b00001000,
    PIC_ICW4_SFNM               = 0b00010000,
} PIC_ICW4;

/** @brief PIC command opcodes. */
enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
} PIC_CMD;

/**
 * @brief Program both PICs with new vector offsets and wiring.
 */
void x86_PIC_Configure(uint8_t offsetPIC1, uint8_t offsetPIC2)
{
    x86_outb(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    x86_iowait();
    x86_outb(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    x86_iowait(); 

    x86_outb(PIC1_DATA_PORT, offsetPIC1);
    x86_iowait();
    x86_outb(PIC2_DATA_PORT, offsetPIC2);
    x86_iowait();

    x86_outb(PIC1_DATA_PORT, 0x04);
    x86_iowait();
    x86_outb(PIC2_DATA_PORT, 0x02);
    x86_iowait();

    x86_outb(PIC1_DATA_PORT, PIC_ICW4_8086);
    x86_iowait();
    x86_outb(PIC2_DATA_PORT, PIC_ICW4_8086);
    x86_iowait();

    x86_outb(PIC1_DATA_PORT, 0);
    x86_iowait();
    x86_outb(PIC2_DATA_PORT, 0);
    x86_iowait();
}

/**
 * @brief Signal end-of-interrupt to the master/slave PIC as appropriate.
 */
void x86_PIC_SendEndOfInterrupt(int irq)
{
    if(irq >= 8)
        x86_outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
    x86_outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
}

/**
 * @brief Mask all PIC IRQ lines.
 */
void x86_PIC_Disable()
{
    x86_outb(PIC1_DATA_PORT, 0xFF);
    x86_iowait();
    x86_outb(PIC2_DATA_PORT, 0xFF);
    x86_iowait();
}

/**
 * @brief Mask a specific IRQ line.
 */
void x86_PIC_Mask(int irq)
{
    uint8_t port;
    if (irq < 8)
        port = PIC1_DATA_PORT;
    else
    {
        irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = x86_inb(port);
    x86_outb(port, mask | (1 << irq));
}

/**
 * @brief Unmask a specific IRQ line.
 */
void x86_PIC_Unmask(int irq)
{
    uint8_t port;
    if (irq < 8)
        port = PIC1_DATA_PORT;
    else
    {
        irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = x86_inb(port);
    x86_outb(port, mask & ~(1 << irq));
}

/**
 * @brief Read the Interrupt Request Register (IRR) from the PICs.
 */
uint16_t x86_PIC_ReadIrqRequestRegister()
{
    x86_outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    x86_outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    return ((uint16_t)x86_inb(PIC1_COMMAND_PORT)) | (((uint16_t)x86_inb(PIC2_COMMAND_PORT)) << 8);
}

/**
 * @brief Read the In-Service Register (ISR) from the PICs.
 */
uint16_t x86_PIC_ReadInServiceRegister()
{
    x86_outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    x86_outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    return ((uint16_t)x86_inb(PIC1_COMMAND_PORT)) | (((uint16_t)x86_inb(PIC2_COMMAND_PORT)) << 8);
}
