#include <stdint.h>

void putchar(const char c);
void scrollback(int lines);
void setcursor(int x, int y);
void putcolor(int x, int y, uint8_t color);
void putchr(int x, int y, char c);
void clrscr();