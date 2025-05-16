#pragma once

void InitSerial();
void SerialWriteChar(char c);
void SerialWriteString(const char* str);
void SerialPrintf(const char* fmt, ...);