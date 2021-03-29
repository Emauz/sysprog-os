/*
*   file:  eth.h
*
*   Intel 8255x Ethernet Device Driver header
*/
#ifndef ETH_H
#define ETH_H

// return values
#define ETH_SUCCESS 0
#define ETH_ERR -1 // general error, can make more specific

#define ETH_VENDOR_ID 0x8086
#define ETH_DEVICE_ID 0x1229 // TODO this depends on which device specifically, check on this

// PCI base address registers (MMIO and IO addr. spaces)
#define ETH_PCI_MM_BAR 0x10
#define ETH_PCI_IO_BAR 0x14

// TODO
void __eth_init();

#endif
