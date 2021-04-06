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
// page 108 for receiving


static void __eth_isr(int vector, int code) {
    // TODO
    // read the SCB status word
    // write a one to that bit when serviced

    // ack all interrupts
    __cio_printf("%04x\n", eth.CSR_IO_BA + ETH_SCB_STATUS_WORD);

    __outb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1, 0xFF);


    #ifdef ETH_DEBUG
    __cio_printf("ETH ISR\n");
    #endif

	__outb(PIC_PRI_CMD_PORT, PIC_EOI);
}

void __eth_init(void) {
    // find the device on the PCI bus
    assert(0 == __pci_find_device(&eth_pci, ETH_VENDOR_ID, ETH_DEVICE_ID));

    // get the BARs
    eth.CSR_IO_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_IO_BAR) & 0xFFF0;
    // eth.CSR_MM_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_MM_BAR);

    // set the device as a PCI master
    uint32_t cmd = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, PCI_CMD_REG_OFFSET);
    cmd |= 0b100; // set bus master bit to 1
    __pci_write32(eth_pci.bus, eth_pci.slot, eth_pci.function, PCI_CMD_REG_OFFSET, cmd);


    #ifdef ETH_DEBUG
    __cio_printf("\nETH IO BA: %x\nETH MMIO BA: %x\n", eth.CSR_IO_BA, eth.CSR_MM_BA);
    __cio_printf("ETH INT_LINE: %02x\n", eth_pci.int_line);
    __cio_printf("ETH CMD REG: %04x\n", __pci_read16(eth_pci.bus, eth_pci.slot, eth_pci.function, PCI_CMD_REG_OFFSET));
    #endif

    // check for any active interrupts and acknowledge them
    __inw(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD);
    __delay(100);
    __outb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1, 0xFF);

    // selective reset
    __outl(eth.CSR_IO_BA + ETH_PORT, ETH_SELECTIVE_RESET);
    __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD);
    __delay(100);

    // soft reset
    __outl(eth.CSR_IO_BA + ETH_PORT, ETH_SOFT_RESET);
    __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD);
    __delay(100); // this delay is longer than needed

    __eth_disable_int();

    // install the ISR on the correct vector number from the PCI config register
    // __install_isr(eth_pci.int_line, &__eth_isr);
    __install_isr(43, &__eth_isr); // magic vector number

    // use linear addressing
    __eth_load_CU_base(0x0);
    __cio_printf("stat: %04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD));
    __eth_load_RU_base(0x0);
    __cio_printf("stat: %04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD));


    // TODO
    // send config command
    // need to set a bit in byte 8 for PHY enable

    __eth_enable_int();

    __cio_printf("eth init done\n");\
}

// disable interrupts
// change M bit in SCB command word MSB
void __eth_disable_int(void) {
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD + 1, 0b1);
}

// enable interrupts w/ M bit in SCB command word MSB
void __eth_enable_int(void) {
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD + 1, 0b0);
}

// load command unit base addr.
void __eth_load_CU_base(uint32_t base_addr) {
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, base_addr);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_LOAD_CU_BASE);

    __cio_printf("load cu base success\n");
}

// load receive unit base
void __eth_load_RU_base(uint32_t base_addr) {
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        #ifdef ETH_DEBUG
        __cio_printf("cmd still executing: %02x\n", cmd_lsb);
        #endif
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, base_addr);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_LOAD_RU_BASE);

    __cio_printf("load ru base success\n");
}

// command unit start
void __eth_CU_start(uint8_t* CBL_Start) {
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        #ifdef ETH_DEBUG
        __cio_printf("cmd still executing: %02x\n", cmd_lsb);
        #endif
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)CBL_Start);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_CU_START);
}

// for TESTING
void __eth_nop(void) {
    __cio_printf("stat: %04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD));
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
