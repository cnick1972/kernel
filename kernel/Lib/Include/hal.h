#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>


#define interrupt

#define far
#define near

extern "C" void _geninterrupt(unsigned int);

extern int hal_initilaize();

extern int hal_shutdown();

extern void geninterrupt(int n);

#endif