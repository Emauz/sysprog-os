/*
*   @file   eth.c
*
*   Intel 8255x Ethernet Device Driver Implementations
*
*   Acronyms:
*     CU = command unit
*     RU = receive unit
*     CBL = command block list (used by the CU)
*     RFA = receive frame area (used by RU)
*     RFD = receive frame desciptor (contained by the RFA)
*     IA = individual address
*
*   Written specifically for the Intel 82557 (does not include more advanced features)
*   Memory modes for the CBL and RFA are both simple mode (not flexible)
*
*   @author Will Merges
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

// MAC initializes to broadcast address on reset
uint64_t _eth_MAC = 0xFFFFFFFFFFFF;

#define CBL_SIZE 8192
#define MAX_COMMANDS 50 // maximum number of commands that can be processed at a time

// max size of an ethernet frame
#define ETH_FRAME_SIZE 1518

// needs to be larger than or equal to ETH_FRAME_SIZE + sizeof(RFD_t)
// pick the next power of 2 just for convenience
#define RFA_SIZE 2048

// action command IA load
typedef struct {
    uint16_t status_word;
    uint16_t cmd_word;
    uint32_t link_addr;
    uint8_t IA_addr_1; // internal address 1st byte
    uint8_t IA_addr_2;
    uint8_t IA_addr_3;
    uint8_t IA_addr_4;
    uint8_t IA_addr_5;
    uint8_t IA_addr_6;
} AddrSetupActionCmd_t;

// action command transmit
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
// frame is placed right after descriptor in simple mode
typedef struct {
    uint16_t status_word;
    uint16_t cmd_word;
    uint32_t link_addr;
    uint32_t _reserved;
    uint16_t count_word;
    uint16_t size_word;
    uint8_t frame[ETH_FRAME_SIZE]; // ethernet frame
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
void (*__eth_cmd_callback)(uint16_t id, uint16_t status) = NULL;

void __eth_set_cmd_callback(void (*callback)(uint16_t id, uint16_t status)) {
    __eth_cmd_callback = callback;
}

// function to be called when a frame is received
void (*__eth_rx_callback)(uint16_t status, const uint8_t* data, uint16_t count);

void __eth_set_rx_callback(void (*callback)(uint16_t status,  const uint8_t* data, uint16_t count)) {
    __eth_rx_callback = callback;
}

// setup a receive frame descriptor
// sets it as the last RFD in the RFA
// uses simple memory mode
static inline void __eth_setup_RFD(RFD_t* RFD) {
    // setup the command word
    RFD->cmd_word = 0;
    RFD->cmd_word |= ETH_RFD_CMD_EL; // set the EL bit to say this is the last RFD in the RFA
    RFD->cmd_word |= ETH_RFD_CMD_SF; // set the SF bit to say we're in simplfified mode

    // zero the link address
    RFD->link_addr = 0;

    // zero the EOF, F, and COUNT bits
    RFD->count_word = 0;

    // set the size byte
    // make sure size is such that bits 6 and 7 are 0
    RFD->size_word = ETH_FRAME_SIZE;
    RFD->size_word &= ~(0b11 << 14);
}

// request a slab of size 'len' of the CBL to place an action command
// returns the pointer to a CBL entry or NULL on no memory available
static inline uint8_t* __eth_allocate_CBL(uint16_t len) {
    // __cio_printf("CBL_end: %04x, CBL_start: %04x, len: %04x\n", CBL_end, CBL_start, len);
    if((CBL_end + len) > CBL_SIZE) {
        CBL_end = 0;
        // wrap around
        if(CBL_end == CBL_start) {
            return NULL;
        }
    }

    if(CBL_end >= CBL_start || CBL_end + len < CBL_start) {
        CBL_end += len;
        CBL_end += (CBL_end % 2); // word align the CBL slab
        return &CBL[CBL_end - len];
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


// internal address should be 48 bits, if it's not an error will be returned
uint8_t __eth_loadaddr(uint64_t addr, uint16_t id) {
    if(addr & ((uint64_t)0xFF << 48)) {
        // address is over 48 bits
        return ETH_TOO_LARGE;
    }

    // update the current MAC address
    _eth_MAC = addr;

    // allocate space on the CBL
    AddrSetupActionCmd_t* ptr = (AddrSetupActionCmd_t*)__eth_allocate_CBL(sizeof(AddrSetupActionCmd_t));
    if(ptr == NULL) {
        #ifdef ETH_DEBUG
        __cio_printf("CBL alloc fail\n");
        #endif

        return ETH_NO_MEM;
    }

    // setup cmd
    ptr->cmd_word = ETH_ACT_CMD_LOAD_ADDR;
    ptr->cmd_word |= ETH_ACT_CMD_EL_MASK;
    // ptr->cmd_word |= ETH_ACT_CMD_I_MASK; // no longer set the I bit
    ptr->IA_addr_6 = addr;
    ptr->IA_addr_5 = addr >> 8;
    ptr->IA_addr_4 = addr >> 16;
    ptr->IA_addr_3 = addr >> 24;
    ptr->IA_addr_2 = addr >> 32;
    ptr->IA_addr_1 = addr >> 40;

    // create a command node
    cmd_node_t* cmd = __eth_allocate_CMD();
    if(cmd == NULL) {
        #ifdef ETH_DEBUG
        __cio_printf("CMD alloc fail\n");
        #endif

        free_commands[cmd->cmd_index] = 0; // free the command node
        CBL_end -= sizeof(AddrSetupActionCmd_t); // unallocate CBL space
        return ETH_NO_MEM;
    }

    cmd->CBL_index = CBL_end - sizeof(AddrSetupActionCmd_t);
    cmd->CBL_size = sizeof(AddrSetupActionCmd_t);
    cmd->id = id;

    if(CU_BUSY) {
        assert(E_SUCCESS == _que_enque(_cu_waiting, cmd, NULL));
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
uint8_t __eth_tx(uint8_t* data, uint16_t len, uint16_t id) {
    // check len is only 14 bits
    if((len >> 14) != 0) {
        return ETH_TOO_LARGE;
    }

    // allocate space on the CBL
    uint8_t* ptr = __eth_allocate_CBL(sizeof(TxActionCmd_t) + len);
    if(ptr == NULL) {
        #ifdef ETH_DEBUG
        __cio_printf("CBL alloc fail\n");
        #endif

        return ETH_NO_MEM;
    }

    // setup cmd
    TxActionCmd_t* TxCB = (TxActionCmd_t*)ptr;
    TxCB->cmd_word = ETH_ACT_CMD_TX;
    TxCB->cmd_word |= ETH_ACT_CMD_EL_MASK;
    // TxCB->cmd_word |= ETH_ACT_CMD_I_MASK; // no longer set the I bit
    TxCB->tbd_array_addr = 0x0; // NULL pointer (in simple mode)
    TxCB->byte_cnt = len;
    TxCB->tx_threshold = 1; // 1 byte in the FIFO triggers a send
    TxCB->TBD_number = 0x0; // doesn't matter in simple mode, zero anyways just in case

    // in simplified mode, the data goes directly after the command block
    __memcpy(TxCB + 1, data, len);

    // create a command node
    cmd_node_t* cmd = __eth_allocate_CMD();
    if(cmd == NULL) {
        #ifdef ETH_DEBUG
        __cio_printf("CMD alloc fail\n");
        #endif

        free_commands[cmd->cmd_index] = 0; // free the command node
        CBL_end -= (sizeof(TxActionCmd_t) + len); // unallocate CBL space
        return ETH_NO_MEM;
    }

    cmd->CBL_index = CBL_end - sizeof(TxActionCmd_t) - len;
    cmd->CBL_size = sizeof(TxActionCmd_t) + len;
    cmd->id = id;

    if(CU_BUSY) {
        assert(E_SUCCESS == _que_enque(_cu_waiting, cmd, NULL));
    } else {
        // start it immediately
        CU_BUSY = 1;
        current_cmd = cmd;
        __eth_CU_start(ptr);
    }

    return ETH_SUCCESS;
}


static void __eth_isr(int vector, int code) {
    // only care about the high byte of the status word (STAT/ACK)
    uint8_t status = __inb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1);

    // ack all interrupts that occured
    // this needs to be read BEFORE trying to execute any CSR commands
    __outb(eth.CSR_IO_BA + ETH_SCB_STATUS_WORD + 1, 0xFF);


    #ifdef ETH_DEBUG
    // print the SCB status word most significant byte
    __cio_printf("%04x\n", status);
    __cio_printf("ETH ISR\n");
    #endif

    if(status & ETH_CX_TNO_MASK) { // CX/TNO interrupt
        #ifdef ETH_DEBUG
        __cio_printf("CX_TNO\n");
        #endif
        // shouldn't happen if we never set the I bit
        // we're not using the TNO functionality of the 82557
        // ignore it and move on
    }

    if(status & ETH_FR_MASK) { // frame ready interrupt
        #ifdef ETH_DEBUG
        __cio_printf("FR INT\n");
        #endif
        // should be called everytime the RU receives a frame

        // call the rx callback function with a pointer to the data section
        // of the last (and only) RFD in the RFA

        // TODO maybe copy out the frame somewhere first and reset the RFD before calling the callback
        // problem with the callback taking too long won't cause an RNR (RU has no space in RFA!)
        // alternatively just make the RFA bigger (e.g. more than one RFD, probably a good idea in the long run)
        if(__eth_rx_callback != NULL) {
            uint16_t actual_count = (RFA->count_word & ~(0b11 << 14)); // ignore top 2 bits

            #ifdef ETH_DEBUG
            __cio_printf("packet: \n");
            for(unsigned int i = 0; i < actual_count; i++) {
                __cio_printf("%02x ", RFA->frame[i]);
            }
            __cio_printf("\n");
            #endif


            if(RFA->status_word & ETH_RFD_STATUS_OK) {
                __eth_rx_callback(ETH_SUCCESS, RFA->frame, actual_count);
            } else {
                __eth_rx_callback(ETH_RECV_ERR, RFA->frame, actual_count);
            }
       }

        // reset the RFD in the RFA
        __eth_setup_RFD(RFA);

        // restart the RU
        __eth_RU_start((uint8_t*)RFA);
    }

    if(status & ETH_CNA_MASK) { // CU not active interrupt
        #ifdef ETH_DEBUG
        __cio_printf("CNA INT\n");
        #endif
        // should happen when tx/loadaddr (or any CU command) finishes

        // uint16_t status = *((uint16_t*)CBL_start);
        // __cio_printf("cmd stat: %04x\n", status);

        // call the callback if it's set
        if(__eth_cmd_callback != NULL) {
            uint8_t status_word_hi = CBL[current_cmd->CBL_index + 1];
            if(status_word_hi & ETH_ACTION_CMD_STATUS_OK) {
                __eth_cmd_callback(current_cmd->id, ETH_SUCCESS);
            } else {
                __eth_cmd_callback(current_cmd->id, ETH_CMD_FAIL);
            }
        }

        // fix the CBL (unallocate the current command block)
        CBL_start = (CBL_start + current_cmd->CBL_size) % CBL_SIZE;
        // move CBL start to a word (2 byte) boundary
        CBL_start += (CBL_start % 2); // this guarantees zero free bytes b/w CBL start and end

        // free the just executed command node (zero the index)
        free_commands[current_cmd->cmd_index] = 0;

        // if we have another command to run we should do it
        if(_que_length(_cu_waiting)) {
            current_cmd = _que_deque(_cu_waiting);
            __eth_CU_start(&CBL[current_cmd->CBL_index]);
        } else {
            current_cmd = NULL;
            // set the CU to not busy
            CU_BUSY = 0;
        }
    }

    if(status & ETH_RNR_MASK) { // RU no resources
        #ifdef ETH_DEBUG
        __cio_printf("RNR INT\n");
        #endif
        // this hopefully never happens, but if it does the RU has no more
        // memory to place frames and the frame was dropped :(
    }

    if(status & ETH_MDI_MASK) { // MDI interrupt (media data interface)
        #ifdef ETH_DEBUG
        __cio_printf("MDI INT\n");
        #endif
        // not implemented
        // should never happen
    }

    if(status & ETH_SWI_MASK) { // software interrupt
        #ifdef ETH_DEBUG
        __cio_printf("SWI int\n");
        #endif
        // not used anywhere
        // should never happen
    }

    // bit 1 is reserved and bit 0 is the FCP bit which is not present on the 82557

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
    RFA = (RFD_t*)RFA_data;
    RFA += ((uint32_t)RFA) % 16; // 16-byte align the RFA
    __eth_setup_RFD(RFA);

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
    // if we do, we should set the NSAI bit in byte 10 of the 82557 config map
    // the default is no source address insertion (won't insert src MAC into ethernet frame header)
    // that would be nice if it was on


    // start the receive unit
    // current plan is to have a single RFD in the RFA with the EL bit always set
    // and every ISR copy out the ethernet frame somewhere (call an RxCallback)
    // and then reset the frame, make sure to check status to send with the callback
    // the interrupt generated should be FR
    __eth_RU_start((uint8_t*)RFA);

    // re-enable interrupts
    __eth_enable_int();

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
