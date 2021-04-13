/*
*   file:  eth.h
*
*   Intel 8255x Ethernet Device Driver header
*/
#ifndef ETH_H
#define ETH_H

#include "common.h"

// return values
#define ETH_SUCCESS 0
#define ETH_ERR 1 // general error, can make more specific
#define ETH_TOO_LARGE 2
#define ETH_NO_MEM 3

// PCI vendor ID and device ID
#define ETH_VENDOR_ID 0x8086
#define ETH_DEVICE_ID 0x1229 // TODO this depends on which device specifically, this is for the 82557

// PCI config. space offsets
#define ETH_PCI_MM_BAR 0x10
#define ETH_PCI_IO_BAR 0x14

// CSR offsets
#define ETH_SCB_STATUS_WORD      0x00
#define ETH_SCB_CMD_WORD         0x02
#define ETH_SCB_GENERAL_POINTER  0x04
#define ETH_PORT                 0x08

// PORT commands
#define ETH_SOFT_RESET       0x00
#define ETH_SELECTIVE_RESET  0x02

// CUC commands (load into SCB LSB)
#define ETH_CU_NOP       (0b0000 << 4)
#define ETH_CU_START     (0b0001 << 4)
#define ETH_CU_RESUME    (0b0010 << 4)
#define ETH_LOAD_CU_BASE (0b0110 << 4)

// RUC commands (load into SCB LSB)
#define ETH_RU_NOP       0b000
#define ETH_RU_START     0b001
#define ETH_RU_RESUME    0b010
#define ETH_LOAD_RU_BASE 0b110

// CSR masks
#define ETH_CMD_SWI_MASK (0x1 << 9)

// action commands
#define ETH_ACT_CMD_EL_MASK   (0b1 << 15)
#define ETH_ACT_CMD_I_MASK    (0b1 << 13)
#define ETH_ACT_CMD_LOAD_ADDR 0b1
#define ETH_LINK_ADDR_OFFSET  0x04
#define ETH_ACT_CMD_TX        0b100


typedef struct {
    uint32_t CSR_MM_BA; // memory mapped base address
    uint32_t CSR_IO_BA; // i/o address space base address (only one of these is necessary)
} eth_dev_t;

// init the ethernet module
void __eth_init(void);
// void __eth_nop(void);

// SCB commands
void __eth_disable_int(void);
void __eth_enable_int(void);
void __eth_load_CU_base(uint32_t base_addr);
void __eth_load_RU_base(uint32_t base_addr);
void __eth_CU_start(uint8_t* CBL_Start);


// ~high-level commands~ //
// return ETH_SUCCESS or other value

// load an internal address into the NIC
// pass an ID to associate the generated command with
uint8_t __eth_loadaddr(uint32_t addr, uint16_t id);

// transmit data of length len
// associate 'id' with the command
uint8_t __eth_tx(uint8_t* data, uint16_t len, uint16_t id);

// receive data of max length len
// associate 'id' with the command
uint8_t __eth_rx(uint8_t* data, uint16_t len, uint16_t id);

// set a function to call when a command is complete
// passes back the id associated with the id and a status of the command
void __eth_setcallback(void (*callback)(uint16_t id, uint16_t status));

#endif
