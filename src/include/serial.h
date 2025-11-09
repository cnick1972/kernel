#pragma once

/**
 * @brief Initialize COM1 for debug output.
 */
void serial_init(void);

/**
 * @brief Write a single character to the serial port.
 */
void serial_write_char(char c);

/**
 * @brief Write a null-terminated string to the serial port.
 */
void serial_write_string(const char* str);

/**
 * @brief Printf-style formatted serial output.
 */
void serial_printf(const char* fmt, ...);
