#include <acpi.h>
#include <stddef.h>
#include <stdint.h>
#include <memory.h>
#include <meminit.h>
#include <stdio.h>

#define RSDP_SEARCH_START   0xc00e0000
#define RSDP_SEARCH_END     0xc0100000
#define RSDP_SEARCH_STEP    16

typedef struct
{
    char Signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdtAddress;
} __attribute__((packed)) rsdp_t;

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
  } __attribute__((packed));

typedef struct
{
    struct ACPISDTHeader h;
    uint32_t PointerToOtherSDT[];
} __attribute__((packed)) RSDT;


void* find_rsdp()
{
    for(uint32_t addr = RSDP_SEARCH_START; addr < RSDP_SEARCH_END; addr += RSDP_SEARCH_STEP) {
        rsdp_t* rsdp = (rsdp_t*)addr;
        int i = memcmp(rsdp->Signature, "RSD PTR ", 8);
        if(!i) {
                char sig[9];
                memset(sig, 0, 9);
                memcpy(sig, rsdp->Signature, 8);
                kprintf("Signature: %s Revision: %d, address: 0x%08x\n", sig, rsdp->revision, rsdp->rsdtAddress);
                MapPhysicalToVirtual((uint8_t*)rsdp->rsdtAddress, (uint8_t*)0xe8000000);

                uint32_t vAddress = 0xe8000000;

                uint8_t* ptr = (uint8_t*)((rsdp->rsdtAddress & 0xfff) + vAddress);

                kprintf("rsdt address: 0x%08x\n", ptr);

                RSDT* rsdt = (RSDT*)ptr;

                uint8_t count = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
                kprintf("Entries %d\n", count);

                for(int j = 0; j < count; j++)
                {
                    char buffer[5];
                    buffer[4] = 0;
                    struct ACPISDTHeader* h = (struct ACPISDTHeader*) ((rsdt->PointerToOtherSDT[j] & 0xfff) + vAddress);
                    memcpy(buffer, h->Signature, 4);
                    kprintf("entry[%d] rsdt: %s\n", j, buffer);
                }


                return (void*)addr;
        }
    }


    return NULL;
}