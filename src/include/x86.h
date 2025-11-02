#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ASMCALL __attribute__((cdecl)) /**< Marker for cdecl assembly linkage. */

/**
 * @brief Write an 8-bit value to an I/O port.
 *
 * @param port  Target I/O port number.
 * @param value Value to write.
 */
void ASMCALL x86_outb(uint16_t port, uint8_t value);

/**
 * @brief Write a 16-bit value to an I/O port.
 *
 * @param port  Target I/O port number.
 * @param value Value to write.
 */
void ASMCALL x86_outw(uint16_t port, uint16_t value);

/**
 * @brief Write a 32-bit value to an I/O port.
 *
 * @param port  Target I/O port number.
 * @param value Value to write.
 */
void ASMCALL x86_outl(uint16_t port, uint32_t value);

/**
 * @brief Read an 8-bit value from an I/O port.
 *
 * @param port Target I/O port number.
 * @return Value read from the port.
 */
uint8_t ASMCALL x86_inb(uint16_t port);

/**
 * @brief Read a 16-bit value from an I/O port.
 *
 * @param port Target I/O port number.
 * @return Value read from the port.
 */
uint16_t ASMCALL x86_inw(uint16_t port);

/**
 * @brief Read a 32-bit value from an I/O port.
 *
 * @param port Target I/O port number.
 * @return Value read from the port.
 */
uint32_t ASMCALL x86_inl(uint16_t port);

/**
 * @brief Halt the CPU after disabling interrupts.
 */
void ASMCALL x86_Panic();

/**
 * @brief Invalidate a single page from the TLB.
 *
 * @param page Virtual page address to invalidate.
 */
void ASMCALL x86_InvalidatePage(uint8_t page);

/**
 * @brief Reload the current page directory (flushes the entire TLB).
 */
void ASMCALL x86_ReloadPageDirectory();

/**
 * @brief Enable CPU interrupts and return previous flags state.
 *
 * @return Previous interrupt flag state.
 */
uint8_t ASMCALL x86_EnableInterrupts();

/**
 * @brief Disable CPU interrupts and return previous flags state.
 *
 * @return Previous interrupt flag state.
 */
uint8_t ASMCALL x86_DisableInterrupts();

/**
 * @brief Retrieve the faulting address from CR2 (used in page faults).
 *
 * @return Current value of CR2.
 */
uint32_t ASMCALL x86_CR2_register();

/**
 * @brief Issue a small delay by writing to an unused port (BUS settle).
 */
void x86_iowait();
