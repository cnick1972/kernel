/**
 * @file system/keyboard/keyboard.c
 * @brief PS/2 keyboard interrupt handler.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <irq.h>
#include <stdio.h>
#include <x86.h>

/** @brief Simple UK keymap for set 1 scancodes. */
static const char uk_keymap[128] = {
    0,   27, '1', '2', '3', '4', '5', '6',    // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=', '\b', '\t', // 0x08 - 0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',   // 0x10 - 0x17
    'o', 'p', '[', ']', '\n', 0,   'a', 's',  // 0x18 - 0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   // 0x20 - 0x27
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',  // 0x28 - 0x2F
    'b', 'n', 'm', ',', '.', '/', 0,   '*',   // 0x30 - 0x37
    0,   ' ', 0,   0,   0,   0,   0,   0,     // 0x38 - 0x3F
    0,   0,   0,   0,   0,   0,   0,   '7',   // 0x40 - 0x47
    '8', '9', '-', '4', '5', '6', '+', '1',   // 0x48 - 0x4F
    '2', '3', '0', '.', 0,   0,   0,   0,     // 0x50 - 0x57
    0,   0,   0,   0,   0,   0,   0,   0      // 0x58 - 0x5F
};

/**
 * @brief Basic PS/2 keyboard interrupt handler.
 */
void keyboard_irq_handler(Registers* regs)
{
    uint8_t scancode;

    scancode = x86_inb(0x60);
    if(scancode < 0x60)
    {
        kprintf("Scancode %c\n", uk_keymap[scancode]);
        //kprintf("Hello Ellie! You pressed the %c key.\n", uk_keymap[scancode]);
    }
}
