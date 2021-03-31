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

    // MAYBE issue a reset command, the manual says to do this since it could be a warm reboot but that will never happen so I think we can not

    // get the BARs
    eth.CSR_IO_BAR = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_IO_BAR);
    eth.CSR_MM_BAR = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_MM_BAR);

    // install the ISR on the correct vector number from the PCI config register
    __install_isr(eth_pci.int_line, &__eth_isr);

    // load 0x00 into CU Base and RU Base to use linear addressing
    // use load CU base and load RU base commands of the CSR
    // ALTERNATIVELY load the start of the tx linked list of commands in CU base and the start of RFD (receive frame data) to RU base (or RU start??) and set CSR general pointer to 0 for all operations

    // enable interrupts
    // write mask to interrupt bits of CSR (found at the CSR BAR)
}
