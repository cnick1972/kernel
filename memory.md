Memory map for os when in paged memory mode.


0x00000000 - 0xbfffffff     - Not used, available for user space
0xc0000000 - 0xc000ffff     - Identy maped stuff, may become available later
0xc0010000 - ??????         - kernel (entry point is 0xc0010000)