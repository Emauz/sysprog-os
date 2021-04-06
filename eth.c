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

#define CBL_SIZE 2048

uint8_t CBL_start[CBL_SIZE];
uint8_t* CBL_end;
uint8_t* CBL;


typedef struct {
    uint16_t status_word;
    uint16_t cmd_word;
    uint32_t link_addr;
    uint32_t IA_addr;
    uint16_t IA_pad; // extra word for longer addresses (no need for ipv4)
} AddrSetupActionCmd_t;

typedef struct {
    // TODO
} TxActionCmd_t;


void __eth_loadaddr(uint32_t addr) {
    // if CU busy, queue up to run after interrupt happens
    // dequeue in ISR and call CU_START
    // TODO

    // setup cmd
    AddrSetupActionCmd_t* ptr = (AddrSetupActionCmd_t*)CBL;
    ptr->cmd_word = ETH_ACT_CMD_LOAD_ADDR;
    ptr->cmd_word |= ETH_ACT_CMD_EL_MASK;
    ptr->cmd_word |= ETH_ACT_CMD_I_MASK;
    ptr->IA_addr = addr;

    __eth_CU_start(CBL);
}


static void __eth_isr(int vector, int code) {
    // ack all interrupts
    __cio_printf("%04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1));

    __outb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1, 0xFF);

    #ifdef ETH_DEBUG
    __cio_printf("ETH ISR\n");
    #endif

	// __outb(PIC_PRI_CMD_PORT, PIC_EOI);
    __outb(PIC_SEC_CMD_PORT, PIC_EOI);

    // __eth_nop();
}

void __eth_init(void) {
    // find the device on the PCI bus
    assert(0 == __pci_find_device(&eth_pci, ETH_VENDOR_ID, ETH_DEVICE_ID));

    // setup CBL
    CBL_end = CBL_start + CBL_SIZE;
    CBL = CBL_start;

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
    // PIC interrupt line given by int_line from PCI, add base offset vector number of PIC to it (0x20)
    __install_isr(eth_pci.int_line + 0x20, &__eth_isr); // magic vector number

    // use linear addressing
    __eth_load_CU_base(0x0);
    __eth_load_RU_base(0x0);

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
