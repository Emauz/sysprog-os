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

    // get the BARs
    eth.CSR_IO_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_IO_BAR);
    eth.CSR_MM_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_MM_BAR);

    #ifdef ETH_DEBUG
    __cio_printf("ETH IO BA: %x\nETH MMIO BA: %x\n", eth.CSR_IO_BA, eth.CSR_MM_BA);
    __cio_printf("ETH INT_LINE: %02x\n", eth_pci.int_line);
    #endif

    // soft reset the device
    *((uint8_t*)eth.CSR_MM_BA + ETH_PORT) = ETH_SOFT_RESET;
    __delay(10); // this delay is longer than needed

    // install the ISR on the correct vector number from the PCI config register
    __install_isr(eth_pci.int_line, &__eth_isr);

    // use linear addressing
    __eth_load_CU_base(0x0);
    __eth_load_RU_base(0x0);
}

// load command unit base addr.
void __eth_load_CU_base(uint32_t base_addr) {
    // set SCB general pointer
    *((uint32_t*)eth.CSR_MM_BA + ETH_SCB_GENERAL_POINTER) = base_addr;

    // uint16_t cmd_word = *((uint16_t*)eth.CSR_MM_BA + ETH_SCB_CMD_WORD);

    // cmd_word = xxxx...xxxxx00110xxx
    // cmd_word |= 0b110000;
    // cmd_word &= 0b00110111;

    // *((uint16_t*)eth.CSR_MM_BA + ETH_SCB_CMD_WORD) = cmd_word;
    *((uint16_t*)eth.CSR_MM_BA + ETH_SCB_CMD_WORD) = 0b110000;
}

// load receive unit base
void __eth_load_RU_base(uint32_t base_addr) {
    // TODO
}

// command unit start
void __eth_CU_start(void) {
    // uint16_t cmd_word = *((uint16_t*)eth.CSR_MM_BA + ETH_SCB_CMD_WORD);

    // cmd_word = xxxx...xxxxx00010xxx
    // cmd_word |= 0b10000;
    // cmd_word &= 0b00010111;

    // *((uint16_t*)eth.CSR_MM_BA + ETH_SCB_CMD_WORD) = cmd_word;
    *((uint16_t*)eth.CSR_MM_BA + ETH_SCB_CMD_WORD) = 0b10000;
}

// for TESTING
void __eth_nop(void) {
    // setup the CBL
    __memset(CBL, 8, 0x0);
    uint8_t nop_cmd = 0b10100000; // set I and EL bit
    CBL[0] = nop_cmd;

    // load CBL addr. into SCB GENERAL ptr.
    *((uint32_t*)eth.CSR_MM_BA + ETH_SCB_GENERAL_POINTER) = (uint32_t)CBL;

    __eth_CU_start();
}
