#include <pci.h>

/**
 * @brief Translate a PCI class code into a human-readable name.
 *
 * @param class_code PCI class code from the device configuration header.
 * @return Pointer to a static string describing the class.
 */
const char* pci_class_name(uint8_t class_code)
{
    switch (class_code) {
    case 0x00: return "Unclassified";
    case 0x01: return "Mass Storage Controller";
    case 0x02: return "Network Controller";
    case 0x03: return "Display Controller";
    case 0x04: return "Multimedia Controller";
    case 0x05: return "Memory Controller";
    case 0x06: return "Bridge Device";
    case 0x07: return "Simple Communication Controller";
    case 0x08: return "Base System Peripheral";
    case 0x09: return "Input Device Controller";
    case 0x0A: return "Docking Station";
    case 0x0B: return "Processor";
    case 0x0C: return "Serial Bus Controller";
    case 0x0D: return "Wireless Controller";
    case 0x0E: return "Intelligent Controller";
    case 0x0F: return "Satellite Communication Controller";
    case 0x10: return "Encryption/Decryption Controller";
    case 0x11: return "Data Acquisition and Signal Processing Controller";
    default:   return "Unknown";
    }
}

/**
 * @brief Translate a PCI subclass/prog-if tuple into a human-readable name.
 *
 * @param class_code PCI class code.
 * @param subclass   PCI subclass code.
 * @param prog_if    Programming interface code.
 * @return Pointer to a static string describing the subclass.
 */
const char* pci_subclass_name(uint8_t class_code, uint8_t subclass, uint8_t prog_if)
{
    switch (class_code) {
    case 0x01: /* Mass Storage Controller */
        switch (subclass) {
        case 0x00: return "SCSI";
        case 0x01: return "IDE";
        case 0x02: return "Floppy";
        case 0x03: return "IPI";
        case 0x04: return "RAID";
        case 0x05: return "ATA";
        case 0x06:
            switch (prog_if) {
            case 0x00: return "SATA (Vendor Specific)";
            case 0x01: return "SATA (AHCI)";
            case 0x02: return "SATA (Serial Storage Bus)";
            default:   return "SATA";
            }
        case 0x07: return "Serial Attached SCSI";
        case 0x08: return "Non-Volatile Memory";
        default:   return "Mass Storage (Other)";
        }
    case 0x02: /* Network */
        switch (subclass) {
        case 0x00: return "Ethernet";
        case 0x01: return "Token Ring";
        case 0x02: return "FDDI";
        case 0x03: return "ATM";
        case 0x04: return "ISDN";
        case 0x07: return "InfiniBand";
        case 0x08: return "Fabric";
        default:   return "Network (Other)";
        }
    case 0x03: /* Display */
        switch (subclass) {
        case 0x00: return "VGA Compatible";
        case 0x01: return "XGA";
        case 0x02: return "3D Controller";
        default:   return "Display (Other)";
        }
    case 0x06: /* Bridge */
        switch (subclass) {
        case 0x00: return "Host Bridge";
        case 0x01: return "ISA Bridge";
        case 0x02: return "EISA Bridge";
        case 0x04: return "PCI-to-PCI Bridge";
        case 0x09: return "PCI Hot-Plug Bridge";
        default:   return "Bridge (Other)";
        }
    case 0x0C: /* Serial Bus */
        switch (subclass) {
        case 0x00: return "FireWire";
        case 0x01: return "ACCESS.bus";
        case 0x02: return "SSA";
        case 0x03:
            switch (prog_if) {
            case 0x00: return "USB (UHCI)";
            case 0x10: return "USB (OHCI)";
            case 0x20: return "USB (EHCI)";
            case 0x30: return "USB (xHCI)";
            case 0x80: return "USB (Unspecified)";
            default:   return "USB";
            }
        case 0x05: return "SMBus";
        default:   return "Serial Bus (Other)";
        }
    default:
        return "Unknown";
    }
}
