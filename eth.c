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
#include "queues.h"

#ifdef ETH_DEBUG
#include "cio.h"
#include "sio.h"
#endif

pci_dev_t eth_pci;
eth_dev_t eth;

uint8_t CU_BUSY = 0; // CU initializes to idle

#define CBL_SIZE 2048
#define MAX_COMMANDS 50 // maximum number of commands that can be processed at a time

typedef struct {
    uint16_t status_word;
    uint16_t cmd_word;
    uint32_t link_addr;
    uint32_t IA_addr;
    uint16_t IA_pad; // extra word for longer addresses (no need for ipv4)
} AddrSetupActionCmd_t;

typedef struct {
    uint16_t status_word;
    uint16_t cmd_word;
    uint32_t link_addr;
    uint32_t tbd_array_addr;
    uint16_t byte_cnt;
    uint8_t tx_threshold;
    uint8_t TBD_number;
} TxActionCmd_t;

// associates an actual command with an id to be returned
typedef struct {
    uint8_t* data; // pointer to command on the CBL
    uint16_t id;   // id of the cmd node (should be set to process PID)
} cmd_node_t;

// statically allocated block of commands to use
cmd_node_t commands[MAX_COMMANDS];
cmd_node_t* cmd_start;
cmd_node_t* cmd_end;
cmd_node_t* current_cmd;

// statically allocated space for action commands to reside
uint8_t CBL_start[CBL_SIZE];
uint8_t* CBL_end;
uint8_t* CBL;

// commands waiting to execute
// holds nodes of type cmd_node_t
queue_t _cu_waiting;


// function to be called when a command is complete, returns the id for whatever command finished
void (*__eth_callback)(uint16_t id, uint16_t status) = NULL;

void __eth_setcallback(void (*callback)(uint16_t id, uint16_t status)) {
    __eth_callback = callback;
}

uint8_t __eth_loadaddr(uint32_t addr, uint16_t id) {
    // setup cmd
    AddrSetupActionCmd_t* ptr = (AddrSetupActionCmd_t*)CBL; // TODO grab the next free space on the CBL
    ptr->cmd_word = ETH_ACT_CMD_LOAD_ADDR;
    ptr->cmd_word |= ETH_ACT_CMD_EL_MASK;
    ptr->cmd_word |= ETH_ACT_CMD_I_MASK;
    ptr->IA_addr = addr;

    // TODO create a command node for it

    if(CU_BUSY) {
        // queue it up
    } else {
        // start it immediately
        // TODO set current_cmd
        __eth_CU_start(CBL);
    }

    return ETH_SUCCESS;
}

// start a transmit command in simple mode
// len must be 14 bits max
uint8_t __eth_tx(uint8_t* data, uint16_t len, uint16_t id) {
    // check len is only 14 bits
    if((len >> 14) != 0) {
        return ETH_TOO_LARGE;
    }

    // setup cmd
    TxActionCmd_t* ptr = (TxActionCmd_t*)CBL; // TODO get the correct space on the CBL
    ptr->cmd_word = ETH_ACT_CMD_TX;
    ptr->cmd_word |= ETH_ACT_CMD_EL_MASK;
    ptr->cmd_word |= ETH_ACT_CMD_I_MASK;
    ptr->tbd_array_addr = 0x0; // NULL pointer (in simple mode)
    ptr->byte_cnt = len;
    ptr->tx_threshold = 1; // 1 byte in the FIFO triggers a send
    ptr->TBD_number = 0x0; // doesn't matter in simple mode, zero anyways just in case

    // in simplified mode, the data goes directly after the command block
    __memcpy(ptr + 1, data, len);

    // TODO create a command node for it


    if(CU_BUSY) {
        // TODO queue it up
    } else {
        // start the CU executing
        // TODO set current_cmd
        __eth_CU_start(CBL);
    }

    return ETH_SUCCESS;
}


static void __eth_isr(int vector, int code) {

    #ifdef ETH_DEBUG
    __cio_printf("%04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1));
    __cio_printf("ETH ISR\n");
    #endif

    // ack all interrupts
    __outb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1, 0xFF);

    // TODO check for certain kinds of interrupts

    // call the callback if it's set
    if(__eth_callback != NULL) {
        // TODO error checking
        __eth_callback(current_cmd->id, ETH_SUCCESS);
    }

    // if we have another command to run we should do it
    if(_que_length(_cu_waiting)) {
        current_cmd = _que_deque(_cu_waiting);
        __eth_CU_start(current_cmd->data);
    } else {
        // set the CU to not busy
        CU_BUSY = 0;
    }

    // acknowledge the SECONDARY PIC
    __outb(PIC_SEC_CMD_PORT, PIC_EOI);
}


void __eth_init(void) {

    __cio_puts(" Eth:");

    // find the device on the PCI bus
    assert(0 == __pci_find_device(&eth_pci, ETH_VENDOR_ID, ETH_DEVICE_ID));

    // setup CBL
    CBL_end = CBL_start + CBL_SIZE;
    CBL = CBL_start;

    // setup command space
    cmd_start = commands;
    cmd_end = commands + MAX_COMMANDS;
    current_cmd = NULL;

    // allocated the queue
    _cu_waiting = _que_alloc(NULL);

    // get the BAR(s)
    eth.CSR_IO_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_IO_BAR) & 0xFFF0;
    // eth.CSR_MM_BA = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, ETH_PCI_MM_BAR);

    // set the device as a PCI master
    uint32_t cmd = __pci_read32(eth_pci.bus, eth_pci.slot, eth_pci.function, PCI_CMD_REG_OFFSET);
    cmd |= 0b100; // set bus master bit to 1
    __pci_write32(eth_pci.bus, eth_pci.slot, eth_pci.function, PCI_CMD_REG_OFFSET, cmd);

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

    // disable interrupts
    __eth_disable_int();

    // install the ISR on the correct vector number from the PCI config register
    // PIC interrupt line given by int_line from PCI, add base offset vector number of PIC to it (0x20)
    __install_isr(eth_pci.int_line + 0x20, &__eth_isr); // magic vector number

    // use linear addressing
    __eth_load_CU_base(0x0);
    __eth_load_RU_base(0x0);

    // TODO send config command?

    // TODO execute RU_START

    // re-enable interrupts
    __eth_enable_int();

    // TODO wait until the CU and RU go into idle mode
    // just to make sure we're good to execute commands in the future

    __cio_puts(" done");
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
    // wait for the last command to be accepted
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, base_addr);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_LOAD_CU_BASE);
}

// load receive unit base
void __eth_load_RU_base(uint32_t base_addr) {
    // wait for the last command to be accepted
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, base_addr);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_LOAD_RU_BASE);
}

// command unit start
void __eth_CU_start(uint8_t* CBL_Start) {
    // wait for the last command to be accepted
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)CBL_Start);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_CU_START);
}

// for TESTING
// void __eth_nop(void) {
//     __cio_printf("stat: %04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD));
//     // setup the CBL
//     __memset(CBL, 8, 0x0);
//     uint8_t nop_cmd = 0b00000101; // set I and EL bit
//     CBL[3] = nop_cmd;
//
//     #ifdef ETH_DEBUG
//     __cio_printf("%02x %02x %02x %02x\n%02x %02x %02x %02x\n",
//                   CBL[0], CBL[1], CBL[2], CBL[3], CBL[4], CBL[5], CBL[6], CBL[7]);
//     #endif
//
//     // load CBL addr. into SCB GENERAL ptr.
//     // *((uint32_t*)eth.CSR_MM_BA + ETH_SCB_GENERAL_POINTER) = (uint32_t)CBL;
//     __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)CBL);
//
//     __eth_CU_start(CBL);
// }
