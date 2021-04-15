/*
*   file:  transport.h
*
*   Transport Later (TL) header file
*/
#ifndef TL_H
#define TL_H

#include "common.h"

// return values
#define TL_SUCCESS 0
#define TL_ERR 1        // general error, can make more specific (see eth.h for reference)
#define TL_TOO_LARGE 2
#define TL_NO_MEM 3

// header lengths
#define UPD_HDR_LEN 64  // 8 bytes, 64 bits

// header values
#define UDP_PROTOCOL    0x11        // 17 in decimal


typedef struct {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t len;       // udp header length + payload length
    uint16_t checksum;
} UDPhdr_t;


// adds a udp header to an ethernet frame.
// data:    payload. This should be a complete transport layer packet (i.e. UPD packet)
// len:     length of the total packet
// pid:     for syscall items
uint8_t* __udp_add_header(uint8_t* data, uint16_t len, pid_t pid);



#endif
