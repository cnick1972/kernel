#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>


#define I86_MAX_INTERRUPTS          256

#define I86_IDT_DESC_BIT16          0x06    //00000110
#define I86_IDT_DESC_DIT32          0x0e    //00001110
#define I86_IDT_DESC_RING1          0x40    //01000000


#endif