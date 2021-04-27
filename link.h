/*
*   file:  link.h
*
*   Link Layer construction
*/
#ifndef LINK_H
#define LINK_H

#include "common.h"
#include "net.h"

#define ETH_PAYLOAD_MIN_SIZE 48

// 14 bytes
#pragma pack(1)
typedef struct {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
} LINKhdr_t;


// adds a layer 2 ethernet frame to a buffer of length len.
// 'msg' is the message to be sent
// return how large the packet is or zero on error
// also adds higher layer encapsulated layers
uint16_t __link_add_header(uint8_t* buff, uint16_t len, msg_t* msg);

// parses a frame into a msg structure
// frame is 'len' bytes at the address 'data'
// will call ipv4 parse frame if etherthype is ipv4, will respond to arp if ethertype is arp type
// returns 1 if the packet needs to be passed to a user, 0 otherwise
int __link_parse_frame(msg_t* msg, uint16_t len, uint8_t* data);

#endif
