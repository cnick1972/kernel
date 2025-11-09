#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <kerndef.h>

/**
 * @brief Write an 8-bit value to an I/O port.
 *
 * @param port  Target I/O port number.
 * @param value Value to write.
 */
void KERNEL_CDECL x86_outb(uint16_t port, uint8_t value);

/**
 * @brief Write a 16-bit value to an I/O port.
 *
 * @param port  Target I/O port number.
 * @param value Value to write.
 */
void KERNEL_CDECL x86_outw(uint16_t port, uint16_t value);

/**
 * @brief Write a 32-bit value to an I/O port.
 *
 * @param port  Target I/O port number.
 * @param value Value to write.
 */
void KERNEL_CDECL x86_outl(uint16_t port, uint32_t value);

/**
 * @brief Read an 8-bit value from an I/O port.
 *
 * @param port Target I/O port number.
 * @return Value read from the port.
 */
uint8_t KERNEL_CDECL x86_inb(uint16_t port);

/**
 * @brief Read a 16-bit value from an I/O port.
 *
 * @param port Target I/O port number.
 * @return Value read from the port.
 */
uint16_t KERNEL_CDECL x86_inw(uint16_t port);

/**
 * @brief Read a 32-bit value from an I/O port.
 *
 * @param port Target I/O port number.
 * @return Value read from the port.
 */
uint32_t KERNEL_CDECL x86_inl(uint16_t port);

/**
 * @brief Halt the CPU after disabling interrupts.
 */
void KERNEL_CDECL x86_panic(void);

/**
 * @brief Invalidate a single page from the TLB.
 *
 * @param page Virtual page address to invalidate.
 */
void KERNEL_CDECL x86_invalidate_page(uint8_t page);

/**
 * @brief Reload the current page directory (flushes the entire TLB).
 */
void KERNEL_CDECL x86_reload_page_directory(void);

/**
 * @brief Enable CPU interrupts and return previous flags state.
 *
 * @return Previous interrupt flag state.
 */
uint8_t KERNEL_CDECL x86_enable_interrupts(void);

/**
 * @brief Disable CPU interrupts and return previous flags state.
 *
 * @return Previous interrupt flag state.
 */
uint8_t KERNEL_CDECL x86_disable_interrupts(void);

/**
 * @brief Retrieve the faulting address from CR2 (used in page faults).
 *
 * @return Current value of CR2.
 */
uint32_t KERNEL_CDECL x86_read_cr2(void);

/**
 * @brief Issue a small delay by writing to an unused port (BUS settle).
 */
void x86_iowait();
