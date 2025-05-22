Memory map for os when in paged memory mode.


0x00000000 - 0xbfffffff     - Not used, available for user space
0xc0000000 - 0xc000ffff     - Identy maped stuff, may become available later
0xc0010000 - ??????         - kernel (entry point is 0xc0010000)

Virtual Memory Map

0x00000000 - 0xbfffffff     - Available User space
0xc0000000 - 0xc0ffffff     - Kernel binary
0xc1000000 - 0xc1003fff     - Console text buffer

0xd0000000 - 0xd0000fff     - heap (temp, will create a better one)
0xe0000000 - 0xfd3fffff     - Video Frame Buffer


0xfff00000 - 0xffffffff     - Page tables
