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
#include "sio.h"
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

    // #ifdef ETH_DEBUG
    // __cio_printf("\nETH IO BA: %x\nETH MMIO BA: %x\n", eth.CSR_IO_BA, eth.CSR_MM_BA);
    // __cio_printf("ETH INT_LINE: %02x\n", eth_pci.int_line);
    // #endif

    // soft reset the device
    // __outb(eth.CSR_IO_BA + ETH_PORT, ETH_SOFT_RESET);
    // __delay(100); // this delay is longer than needed

    __disable_int();

    // install the ISR on the correct vector number from the PCI config register
    __install_isr(eth_pci.int_line, &__eth_isr);

    // use linear addressing
    __eth_load_CU_base(0x0);
    __eth_load_RU_base(0x0);

    #ifdef ETH_DEBUG
    __cio_printf("eth init done\n");
    #endif

    // send config command
    // need to set a bit in byte 8 for PHY enable

    __enable_int();
}

// disable interrupts
// change M bit in SCB command word MSB
void __disable_int(void) {
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD + 1, 0b10);
}

// enable interrupts w/ M bit in SCB command word MSB
void __enable_int(void) {
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD + 1, 0b00);
}

// load command unit base addr.
void __eth_load_CU_base(uint32_t base_addr) {
    uint8_t cmd_lsb;
    while((cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD))) {
        #ifdef ETH_DEBUG
        __cio_printf("cmd still executing: %02x\n", cmd_lsb);
        __delay(100);
        #endif
    }

    #ifdef ETH_DEBUG
    __cio_printf("load cu base\n");
    #endif

    // set SCB general pointer
    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, base_addr);

    // execute load CU base SCB command
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_LOAD_CU_BASE);
}

// load receive unit base
void __eth_load_RU_base(uint32_t base_addr) {
    uint8_t cmd_lsb;
    while((cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD))) {
        #ifdef ETH_DEBUG
        __cio_printf("cmd still executing: %02x\n", cmd_lsb);
        __delay(100);
        #endif
    }

    #ifdef ETH_DEBUG
    __cio_printf("load ru base\n");
    #endif

    // set SCB general pointer
    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, base_addr);

    // execute load RU base SCB command
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_LOAD_RU_BASE);
}

// command unit start
void __eth_CU_start(uint8_t* CBL_Start) {
    uint8_t cmd_lsb;
    while((cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD))) {
        #ifdef ETH_DEBUG
        __cio_printf("cmd still executing: %02x\n", cmd_lsb);
        __delay(100);
        #endif
    }

    #ifdef ETH_DEBUG
    __cio_printf("CU start\n");
    #endif

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)CBL_Start);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_CU_START);
}

// for TESTING
void __eth_nop(void) {
    // setup the CBL
    __memset(CBL, 8, 0x0);
    uint8_t nop_cmd = 0b00000101; // set I and EL bit
    CBL[3] = nop_cmd;

    #ifdef ETH_DEBUG
    __cio_printf("%02x %02x %02x %02x\n%02x %02x %02x %02x\n",
                  CBL[0], CBL[1], CBL[2], CBL[3], CBL[4], CBL[5], CBL[6], CBL[7]);
    #endif

    // load CBL addr. into SCB GENERAL ptr.
    // *((uint32_t*)eth.CSR_MM_BA + ETH_SCB_GENERAL_POINTER) = (uint32_t)CBL;
    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)CBL);

    __eth_CU_start(CBL);
}
