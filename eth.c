/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "eth.h"
#include "pci.h"

pci_dev_t eth_pci;

void __eth_init() {
    assert(0 == __pci_find_device(&eth_dev, ETH_VENDOR_ID, ETH_DEVICE_ID));
}
