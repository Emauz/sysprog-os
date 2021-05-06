/*
*   @file:  eth.h
*
*   Intel 8255x Ethernet Device Driver header
*
*   @author Will Merges
*/
#ifndef ETH_H
#define ETH_H

#include "common.h"

// return values
#define ETH_SUCCESS 0
#define ETH_ERR 1 // general error, can make more specific
#define ETH_TOO_LARGE 2
#define ETH_NO_MEM 3
#define ETH_RECV_ERR 4
#define ETH_CMD_FAIL 5

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

// action commands
#define ETH_ACT_CMD_EL_MASK   (0b1 << 15)
#define ETH_ACT_CMD_I_MASK    (0b1 << 13)
#define ETH_ACT_CMD_LOAD_ADDR 0b1
#define ETH_LINK_ADDR_OFFSET  0x04
#define ETH_ACT_CMD_TX        0b100

// action command status word high byte mask
#define ETH_ACTION_CMD_STATUS_OK (1 << 5)

// receive frame desciptor status word
#define ETH_RFD_STATUS_OK (1 << 13)
#define ETH_RFD_STATUS_ANY_ERROR 0b0001111111111111 // bits 0-12 set on any error

// CSR Status Word MSB interrupt masks
#define ETH_SWI_MASK (1 << 2)
#define ETH_MDI_MASK (1 << 3)
#define ETH_RNR_MASK (1 << 4)
#define ETH_CNA_MASK (1 << 5)
#define ETH_FR_MASK  (1 << 6)
#define ETH_CX_TNO_MASK (1 << 7)

// RFD command word bits
#define ETH_RFD_CMD_EL (1 << 31)
#define ETH_RFD_CMD_SF (1 << 19)


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
void __eth_CU_start(uint8_t* CBL_addr);
void __eth_RU_start(uint8_t* RFA_addr);

// holds the current MAC address of the NIC
extern uint64_t _eth_MAC;

// ~high-level commands~ //
// return ETH_SUCCESS or other value

// load an internal (MAC) address into the NIC
// address must be 48-bits, if it's not will return ETH_TOO_LARGE
// pass an ID to associate the generated command with
uint8_t __eth_loadaddr(uint64_t addr, uint16_t id);

// transmit data of length len
// associate 'id' with the command
uint8_t __eth_tx(uint8_t* data, uint16_t len, uint16_t id);

// receive data of max length len
// associate 'id' with the command
uint8_t __eth_rx(uint8_t* data, uint16_t len, uint16_t id);

// set a function to call when a command is complete
// passes back the id associated with the id and a status of the command
void __eth_set_cmd_callback(void (*callback)(uint16_t id, uint16_t status));

// set a function to call when a packet is received
// passes back id, status, a pointer to the packet, and a count of how many bytes are in the packet
// CANNOT modify the data at 'data', it needs to be copied out
void __eth_set_rx_callback(void (*callback)(uint16_t status,  const uint8_t* data, uint16_t count));

#endif
