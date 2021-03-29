/*
*   file:  pci.h
*
*   PCI bus utility library
*/

#ifndef PCI_H
#define PCI_H

#include <common.h>


#define PCI_CONFIG_ADDR_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT 0xCFC
#define PCI_VENDOR_ID_OFFSET 0
#define PCI_DEVICE_ID_OFFSET 2

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
} pci_dev_t;


// read n bits from PCI device
uint8_t pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

// fill in dev w/ PCI info for device matching vendorID and deviceID
// checks all possible PCI addresses for a device with matching ID's
// returns 0 on success, -1 on failure
int pci_find_device(pci_dev_t* dev, uint16_t vendor_id, uint16_t device_id);

#endif
