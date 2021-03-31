/*
*   file: pci.c
*
*   PCI bus access functions
*/
#include "pci.h"
#include "klib.h"

// (almost) directly from OS dev wiki
// https://wiki.osdev.org/PCI
uint16_t __pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    __outl(PCI_CONFIG_ADDR_PORT, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint16_t)((__inl(PCI_CONFIG_DATA_PORT) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint32_t __pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    __outl(PCI_CONFIG_ADDR_PORT, address);
    return __inl(PCI_CONFIG_DATA_PORT);
}

uint8_t __pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    __outl(PCI_CONFIG_ADDR_PORT, address);

    // TODO check if this is correct
    tmp = (uint16_t)((__inl(PCI_CONFIG_DATA_PORT) >> ((offset & 0b11) * 8)) & 0xffff);
    return (tmp);
}

int __pci_find_device(pci_dev_t* dev, uint16_t vendorID, uint16_t deviceID) {
    for(int bus = 0; bus < 256; bus++) {
        for(int slot = 0; slot < 32; slot++) {
            // for now assume all devices have function = 0 (don't care about bridges etc.)
            if(vendorID == __pci_read16(bus, slot, 0, PCI_VENDOR_ID_OFFSET)) {
                if(deviceID == __pci_read16(bus, slot, 0, PCI_DEVICE_ID_OFFSET)) {
                    dev->bus = bus;
                    dev->slot = slot;
                    dev->function = 0;
                    dev->int_line = __pci_read8(bus, slot, 0, PCI_INT_LINE_OFFSET);
                    return 0;
                }
            }
        }
    }
    return 1;
}
