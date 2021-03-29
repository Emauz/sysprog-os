/*
*   file: pci.c
*
*   PCI bus access functions
*/
#include "pci.h"

// (almost) directly from OS dev wiki
// https://wiki.osdev.org/PCI
uint16_t pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(PCI_CONFIG_ADDR_PORT, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint16_t)((inl(PCI_CONFIG_DATA_PORT) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDR_PORT, address);
    return (uint16_t)inl(PCI_CONFIG_DATA_PORT);
}

uint32_t pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDR_PORT, address);

    // TODO check if this is correct
    tmp = (uint16_t)((inl(PCI_CONFIG_DATA_PORT) >> ((offset & 0b11) * 8)) & 0xffff);
    return (tmp);
}

int pci_find_device(pci_dev_t* dev, uint16_t vendorID, uint16_t deviceID) {
    uint16_t v_id;
    uint16_t d_id;
    for(int bus = 0; bus < 256; bus++) {
        for(int slot = 0; slot < 32; slot++) {
            // assume all devices have function = 0 (don't care about bridges etc.)
            if(vendorID == pci_read16(bus, slot, 0, PCI_VENDOR_ID_OFFSET)) {
                if(deviceID == pci_read16(bus, slot, 0, PCI_DEVICE_ID_OFFSET)) {
                    dev->bus = bus;
                    dev->slot = slot;
                    dev->func = 0; // TODO for now we only check function = 0
                    return 0;
                }
            }
        }
    }
    return 1;
}
