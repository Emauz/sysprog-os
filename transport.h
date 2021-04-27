/*
*   file:  transport.h
*
*   Transport Later (TL) header file
*/
#ifndef TL_H
#define TL_H

#include "common.h"
#include "net.h"

// return values
#define TL_SUCCESS 0
#define TL_ERR 1        // general error, can make more specific (see eth.h for reference)
#define TL_TOO_LARGE 2
#define TL_NO_MEM 3

// header lengths
#define UPD_HDR_LEN 64  // 8 bytes, 64 bits

// header values
#define UDP_PROTOCOL    0x11        // 17 in decimal

// 8 bytes
#pragma pack(1)
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t len[2];       // udp header length + payload length
    uint16_t checksum;
} UDPhdr_t;


// adds a UDP header and paylaod to a buffer of length len.
// 'msg' is the message to be sent
// return how large the packet is or 0 on error
uint16_t __udp_add_header(uint8_t* buff, uint16_t len, msg_t* msg);

// parses a frame into a msg structure
// frame is 'len' bytes at the address 'data'
// will copy as much of UDP payload into msg->data as can fit
// returns 1 if the packet needs to be passed to a user, 0 on error
int __udp_parse_frame(msg_t* msg, uint16_t len, uint8_t* data);

#endif
