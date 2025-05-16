#include <x86.h>
#include <stdarg.h>
#include <stdio.h>

#define COM1 0x3f8

void InitSerial()
{
    x86_outb(COM1 +1, 0x00);
    x86_outb(COM1 +3, 0x80);
    x86_outb(COM1 +0, 0x03);
    x86_outb(COM1 +1, 0x00);
    x86_outb(COM1 +3, 0x03);
    x86_outb(COM1 +2, 0xc7);
    x86_outb(COM1 +4, 0x0b);      
}

uint8_t SerialIsTransmitEmpty()
{
    return x86_inb(COM1 + 5) & 0x20;
}

void SerialWriteChar(char c)
{
    while(!SerialIsTransmitEmpty());

    x86_outb(COM1, c);
}

void SerialWriteString(const char* str) {
    while (*str) {
        if (*str == '\n') SerialWriteChar('\r'); // For proper line endings
        SerialWriteChar(*str++);
    }
}

void SerialPrintf(const char* fmt, ...) 
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    SerialWriteString(buf);
}
