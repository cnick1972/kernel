#pragma once

/**
 * @brief Initialize COM1 for debug output.\n+ */
void InitSerial();

/**
 * @brief Write a single character to the serial port.
 */
void SerialWriteChar(char c);

/**
 * @brief Write a null-terminated string to the serial port.
 */
void SerialWriteString(const char* str);

/**
 * @brief Printf-style formatted serial output.
 */
void SerialPrintf(const char* fmt, ...);
