/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "eth.h"
#include "pci.h"
#include "support.h"

pci_dev_t eth_pci;

void __eth_init() {
    // find the device on the PCI bus
    assert(0 == __pci_find_device(&eth_dev, ETH_VENDOR_ID, ETH_DEVICE_ID));

    // install the ISR on the correct vector number from the PCI config register
    __install_isr(eth_pci.int_line, &__eth_isr);

    // maybe initialize to a default IP address, or maybe set that in a syscall?? TODO
}
