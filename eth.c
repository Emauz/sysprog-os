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

#define CBL_SIZE 8192
#define MAX_COMMANDS 50 // maximum number of commands that can be processed at a time

// max ethernet frame size is 1518 bytes
// pick the next power of 2 just for ease
#define RFA_SIZE 16384

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
    uint16_t CBL_index; // index to command in the CBL
    uint16_t CBL_size;      // size of the action command on the CBL
    uint16_t cmd_index; // index into 'commands' block
    uint16_t id;        // id of the cmd node (should be set to process PID)
} cmd_node_t;

// Receive Frame Descriptor
// frame is placed right after descirptor in simple mode
typedef struct {
    uint16_t status_word;
    uint16_t cmd_word;
    uint32_t link_add;
    uint32_t _reserved;
    uint16_t count_byte;
    uint16_t size_byte;
    uint8_t frame[1518]; // ethernet frame
} RFD_t;

// statically allocated block of commands to use
cmd_node_t commands[MAX_COMMANDS];
uint8_t free_commands[MAX_COMMANDS]; // bit map of open indices in 'commands'
cmd_node_t* current_cmd;

// statically allocated space for action commands to reside
uint8_t CBL_data[CBL_SIZE];
uint8_t* CBL;
int CBL_start; // index into CBL
int CBL_end; // index into CBL

// commands waiting to execute
// holds nodes of type cmd_node_t
queue_t _cu_waiting;

// receive frame area
uint8_t* RFA_data[RFA_SIZE];
RFD_t* RFA;


// function to be called when a command is complete, returns the id for whatever command finished
void (*__eth_callback)(uint16_t id, uint16_t status) = NULL;

void __eth_setcallback(void (*callback)(uint16_t id, uint16_t status)) {
    __eth_callback = callback;
}

// request a slab of size 'len' of the CBL to place an action command
// returns the pointer to a CBL entry or NULL on no memory available
static inline uint8_t* __eth_allocate_CBL(uint16_t len) {
    if((CBL_end + len) > CBL_SIZE) {
        CBL_end = 0;
        // wrap around
        if(CBL_end == CBL_start) {
            return NULL;
        }
    }

    if(CBL_end >= CBL_start || CBL_end + len < CBL_start) {
        CBL_end += (CBL_end % 2); // word align the CBL slab
        CBL_end += len;
        return &CBL[CBL_end - len]; // TODO 2 byte align this?
    }
    return NULL;
}

// allocate a command node
static inline cmd_node_t* __eth_allocate_CMD(void) {
    for(int i = 0; i < MAX_COMMANDS; i++) {
        if(free_commands[i] == 0) { // we found a free index!
            free_commands[i] = 1;
            commands[i].cmd_index = i;
            return &commands[i];
        }
    }
    return NULL;
}


uint8_t __eth_loadaddr(uint32_t addr, uint16_t id) {
    // allocate space on the CBL
    AddrSetupActionCmd_t* ptr = (AddrSetupActionCmd_t*)__eth_allocate_CBL(sizeof(AddrSetupActionCmd_t));
    if(ptr == NULL) {
        return ETH_NO_MEM;
    }

    // setup cmd
    ptr->cmd_word = ETH_ACT_CMD_LOAD_ADDR;
    ptr->cmd_word |= ETH_ACT_CMD_EL_MASK;
    ptr->cmd_word |= ETH_ACT_CMD_I_MASK;
    ptr->IA_addr = addr;

    // create a command node
    cmd_node_t* cmd = __eth_allocate_CMD();
    if(cmd == NULL) {
        free_commands[cmd->cmd_index] = 0; // free the command node
        CBL_end -= sizeof(AddrSetupActionCmd_t); // unallocate CBL space
        return ETH_NO_MEM;
    }

    cmd->CBL_index = CBL_end - sizeof(AddrSetupActionCmd_t);
    cmd->CBL_size = sizeof(AddrSetupActionCmd_t);

    if(CU_BUSY) {
        _que_enque(_cu_waiting, cmd, NULL); // TODO assert this succeeds
    } else {
        // start it immediately
        CU_BUSY = 1;
        current_cmd = cmd;
        __eth_CU_start((uint8_t*)ptr);
    }

    return ETH_SUCCESS;
}

// start a transmit command in simple mode
// len must be 14 bits max
// Includes PID of transmitting process (to ensure queue synchronization)
uint8_t __eth_tx(uint8_t* data, uint16_t len, pid_t pid) {
    // check len is only 14 bits
    if((len >> 14) != 0) {
        return ETH_TOO_LARGE;
    }

    // allocate space on the CBL
    uint8_t* ptr = __eth_allocate_CBL(sizeof(TxActionCmd_t) + len);
    if(ptr == NULL) {
        __cio_printf("CBL alloc fail\n");
        return ETH_NO_MEM;
    }
    __cio_printf("CBL: %08x", (uint32_t)ptr);

    // setup cmd
    TxActionCmd_t* TxCB = (TxActionCmd_t*)ptr;
    TxCB->cmd_word = ETH_ACT_CMD_TX;
    TxCB->cmd_word |= ETH_ACT_CMD_EL_MASK;
    TxCB->cmd_word |= ETH_ACT_CMD_I_MASK;
    TxCB->tbd_array_addr = 0x0; // NULL pointer (in simple mode)
    TxCB->byte_cnt = len;
    TxCB->tx_threshold = 1; // 1 byte in the FIFO triggers a send
    TxCB->TBD_number = 0x0; // doesn't matter in simple mode, zero anyways just in case

    // in simplified mode, the data goes directly after the command block
    __memcpy(TxCB + 1, data, len);

    // create a command node
    cmd_node_t* cmd = __eth_allocate_CMD();
    if(cmd == NULL) {
        __cio_printf("CMD alloc fail\n");

        free_commands[cmd->cmd_index] = 0; // free the command node
        CBL_end -= (sizeof(TxActionCmd_t) + len); // unallocate CBL space
        return ETH_NO_MEM;
    }

    cmd->CBL_index = CBL_end - sizeof(TxActionCmd_t) - len;
    cmd->CBL_size = sizeof(TxActionCmd_t) + len;

    __cio_printf("CMD: %08x", (uint32_t)cmd);

    __cio_printf("tx happening\n");

    if(CU_BUSY) {
        __cio_printf("CU BUSY, queue instead\n");
        _que_enque(_cu_waiting, cmd, NULL); // TODO assert this succeeds
    } else {
        __cio_printf("tx immediate\n");
        // start it immediately
        CU_BUSY = 1;
        current_cmd = cmd;
        __eth_CU_start(ptr);
    }

    return ETH_SUCCESS;
}


static void __eth_isr(int vector, int code) {
    // TODO check for certain kinds of interrupts

    // all receives will generate RU out of resources (I think) since the EL bit is set
    // need to check status in RFD (page 101)

    #ifdef ETH_DEBUG
    __cio_printf("%04x\n", __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1));
    __cio_printf("ETH ISR\n");
    #endif

    // ack all interrupts
    __outb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1, 0xFF);

    // call the callback if it's set
    if(__eth_callback != NULL) {
        // TODO error checking
        __eth_callback(current_cmd->id, ETH_SUCCESS);
    }

    // fix the CBL (only for transmit and loadaddr)
    CBL_start = (CBL_start + current_cmd->CBL_size) % CBL_SIZE;
    // move CBL start to a word (2 byte) boundary
    CBL_start += (CBL_start % 2); // this guarantees zero free bytes b/w CBL start and end

    // free the just executed command node (zero the index)
    free_commands[current_cmd->cmd_index] = 0;

    // if we have another command to run we should do it
    if(_que_length(_cu_waiting)) {
        __cio_printf("ISR dequeue\n");
        current_cmd = _que_deque(_cu_waiting);
        __eth_CU_start(&CBL[current_cmd->CBL_index]);
    } else {
        __cio_printf("nothing to dequeue\n");
        current_cmd = NULL;
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
    CBL = CBL_data;
    CBL += ((uint32_t)CBL) % 2; // make sure CBL is word alignmed
    CBL_start = 0;
    CBL_end = 0;

    // setup RFA
    RFA = RFA_data;
    RFA += ((uint32_t)RFA) % 16; // 16-byte align the RFA

    // setup command space
    __memset(free_commands, MAX_COMMANDS, 0x0);
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
    // we can assume CU and RU are idle since we just bonked the card
    __eth_load_CU_base(0x0);
    __eth_load_RU_base(0x0);

    // TODO send config command?


    // re-enable interrupts
    __eth_enable_int();

    // start the receive unit
    // TODO set up the RFD in the RFA
    // current plan is to have a single RFD in the RFA with the EL bit always set
    // and every ISR copy out the ethernet frame somewhere (call an RxCallback??)
    // and then reset the frame, make sure to check status to send with the callback
    // the interrupt generated by the EL bit being set should be RU out of resources
    __eth_RU_start(RFA);

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
void __eth_CU_start(uint8_t* CBL_addr) {
    // wait for the last command to be accepted
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)CBL_addr);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_CU_START);
}

// receive unit start
void __eth_RU_start(uint8_t* RFA_addr) {
    // technically we don't have to wait for this since we're commanding the RU unit
    // wait for the last command to be accepted
    uint8_t cmd_lsb =  __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
    while(cmd_lsb) {
        cmd_lsb = __inb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD);
        __delay(100);
    }

    __outl(eth.CSR_IO_BA + ETH_SCB_GENERAL_POINTER, (uint32_t)RFA_addr);
    __outb(eth.CSR_IO_BA + ETH_SCB_CMD_WORD, ETH_RU_START);
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
