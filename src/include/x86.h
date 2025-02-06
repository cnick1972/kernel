#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ASMCALL __attribute__((cdecl))

void        ASMCALL x86_outb(uint16_t port, uint8_t value);
void        ASMCALL x86_outw(uint16_t port, uint16_t value);
void        ASMCALL x86_outl(uint16_t port, uint32_t value);

uint8_t     ASMCALL x86_inb(uint16_t port);
uint16_t    ASMCALL x86_inw(uint16_t port);
uint32_t    ASMCALL x86_inl(uint16_t port);

void        ASMCALL x86_Panic();

void        ASMCALL x86_InvalidatePage(uint8_t page);
void        ASMCALL x86_ReloadPageDirectory();

uint8_t     ASMCALL x86_EnableInterrupts();
uint8_t     ASMCALL x86_DisableInterrupts();

void x86_iowait();