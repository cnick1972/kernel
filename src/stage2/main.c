#include <stdint.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "memdetect.h"
#include "../libs/boot/bootparams.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams g_BootParams;


typedef void (*KernelStart)(BootParams* bootParams);

void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    clrscr();

    DISK disk, disk2;
    if (!DISK_Initialize(&disk, bootDrive))
    {
        printf("Disk init error\r\n");
        goto end;
    }

    if (!FAT_Initialize(&disk))
    {
        printf("FAT init error\r\n");
        goto end;
    }

    // load kernel
    FAT_File* fd = FAT_Open(&disk, "/kernel.bin");
    uint32_t read;
    uint8_t* kernelBuffer = Kernel;
    while ((read = FAT_Read(&disk, fd, MEMORY_LOAD_SIZE, KernelLoadBuffer)))
    {
        memcpy(kernelBuffer, KernelLoadBuffer, read);
        kernelBuffer += read;
    }
    FAT_Close(fd);

    g_BootParams.BootDevice = bootDrive;
    printf("Detecting memory\n");
    MemoryDetect(&g_BootParams.Memory);

    // execute kernel
    KernelStart kernelStart = (KernelStart)Kernel;
    kernelStart(&g_BootParams);

end:
    for (;;);
}
