#include <framebuffer.h>
#include <meminit.h>
#include <x86.h>
#include <stddef.h>

uint32_t TotalPagesRequired;
Framebuffer fb = {NULL, 0, 0, 0, 0};

void InitFramebuffer(multiboot_info* mbi)
{
    fb.address = (void*)(uint32_t)mbi->framebuffer_addr;
    fb.height = mbi->framebuffer_height;
    fb.width = mbi->framebuffer_width;
    fb.bpp = mbi->framebuffer_bpp;
    fb.pitch = mbi->framebuffer_pitch;

    uint32_t FrameBufferVirtualAddress = 0xe0000000;


    // currently the fb.address is a physical address, it needs to be mapped to
    // a virtual address space.  Convention for x86 is at address 0xe0000000.

    // First part is to calculate how many 4k pages are required.  This is foung by multiplying the
    // height by width to calculate the number of pixels, the multiplying by the bytes per pixels.
    // bytes per pixels is calculated by dividing bpp by 8, of shift 3 bit right.

    // We should just alocate 4MB pages for video memory

    int BytesPerPixel = fb.bpp >> 3;
    uint32_t TotalScreenPixels = fb.height * fb.width;
    TotalPagesRequired = (TotalScreenPixels * BytesPerPixel) / 4096;
    if((TotalScreenPixels * BytesPerPixel) % 4096)
        TotalPagesRequired++;

    uint8_t FourMBPagesRequied = TotalPagesRequired / 1024;
    if(TotalPagesRequired % 1024)
        FourMBPagesRequied++;

    Map4MBPhysicalToVirtual(fb.address, (uint8_t*)FrameBufferVirtualAddress, FourMBPagesRequied * 4);
    
    x86_ReloadPageDirectory();

    // set the framebuffer address in the framebuffer structure to the virtual address

    fb.address = (void*)FrameBufferVirtualAddress;
}