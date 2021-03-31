/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "eth.h"
#include "pci.h"
#include "support.h"
#include "common.h"

pci_dev_t eth_pci;
eth_dev_t eth;

static void __eth_isr(int vector, int code) {
    // TODO
}

void __eth_init() {
    // find the device on the PCI bus
    assert(0 == __pci_find_device(&eth_dev, ETH_VENDOR_ID, ETH_DEVICE_ID));

    // get the BARs
    eth.CSR_IO_BAR = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_IO_BAR);
    eth.CSR_MM_BAR = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_MM_BAR);

    // install the ISR on the correct vector number from the PCI config register
    __install_isr(eth_pci.int_line, &__eth_isr);

    // load 0x00 into CU Base and RU Base to use linear addressing
    // use load CU base and load RU base commands of the CSR

    // enable interrupts
    // write mask to interrupt bits of CSR (found at the CSR BAR)
}
