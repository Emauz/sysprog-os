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

#define PCI_VENDOR_ID_OFFSET 0x00
#define PCI_DEVICE_ID_OFFSET 0x02
#define PCI_INT_LINE_OFFSET  0x3C // interrupt vector number that comes in on the PIC
#define PCI_CMD_REG_OFFSET   0x04

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    uint8_t int_line; // interrupt line
} pci_dev_t;


// read n bits from PCI device
uint8_t __pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t __pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t __pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

// write n bits to PCI device
void __pci_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t data);

// fill in dev w/ PCI info for device matching vendorID and deviceID
// checks all possible PCI addresses for a device with matching ID's
// returns 0 on success, -1 on failure
int __pci_find_device(pci_dev_t* dev, uint16_t vendor_id, uint16_t device_id);

#endif
