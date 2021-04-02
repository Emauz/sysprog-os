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
#define ETH_ERR -1 // general error, can make more specific

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
#define ETH_SOFT_RESET  0x00

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

// CU action commands
// TODO


typedef struct {
    uint32_t CSR_MM_BA; // memory mapped base address
    uint32_t CSR_IO_BA; // i/o address space base address (only one of these is necessary)
} eth_dev_t;

// init the ethernet module
void __eth_init(void);
void __eth_nop(void);

// SCB commands
void __enable_scb_swi(void);
void __eth_load_CU_base(uint32_t base_addr);
void __eth_load_RU_base(uint32_t base_addr);
void __eth_CU_start(uint8_t* CBL_Start);

#endif
