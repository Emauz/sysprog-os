/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "eth.h"
#include "pci.h"
#include "support.h"
#include "common.h"
#include "kdefs.h"
#include "x86pic.h"
#include "klib.h"

#ifdef ETH_DEBUG
#include "cio.h"
#endif

pci_dev_t eth_pci;
eth_dev_t eth;


uint8_t CBL[2048];

// TX:
// load CU Base ADDR (probably 0x0)
// setup CBL w/ a transmit command (set interrupt flag for last block or NOP)
// execute CU Start from the SCB (in the CSR) w/ SCB general pointer = CBL start addr.
// profit

static void __eth_isr(int vector, int code) {
    // TODO
    // read the SCB status word
    // write a one to that bit when serviced

    #ifdef ETH_DEBUG
    __cio_printf("ETH ISR\n");
    #endif

	__outb(PIC_PRI_CMD_PORT, PIC_EOI);
}

// page 108 for receiving

void __eth_init(void) {
    // find the device on the PCI bus
    assert(0 == __pci_find_device(&eth_pci, ETH_VENDOR_ID, ETH_DEVICE_ID));

    // MAYBE issue a reset command, the manual says to do this since it could be a warm reboot but that will never happen so I think we can not

    // get the BARs
    eth.CSR_IO_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_IO_BAR);
    eth.CSR_MM_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_MM_BAR);

    #ifdef ETH_DEBUG
    __cio_printf("ETH IO BA: %x\nETH MMIO BA: %x\n", eth.CSR_IO_BA, eth.CSR_MM_BA);
    __cio_printf("ETH INT_LINE: %02x\n", eth_pci.int_line);
    #endif

    // install the ISR on the correct vector number from the PCI config register
    __install_isr(eth_pci.int_line, &__eth_isr);

    // load 0x00 into CU Base and RU Base to use linear addressing
    // use load CU base and load RU base commands of the CSR
    // ALTERNATIVELY load the start of the tx linked list of commands in CU base and the start of RFD (receive frame data) to RU base (or RU start??) and set CSR general pointer to 0 for all operations

    // enable interrupts
    // write mask to interrupt bits of CSR (found at the CSR BAR)
}

// for TESTING
void __eth_nop(void) {
    // setup the CBL
    __memset(CBL, 8, 0x0);
    uint8_t nop_cmd = 0b10100000; // set I and EL bit
    CBL[0] = nop_cmd;

    // CU Start command
    *((uint16_t*)eth.CSR_MM_BA) &= (0x1 << 4);
}
