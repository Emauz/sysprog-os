/*
*   @file   arp.h
*
*   ARP protocol declarations
*
*   @author Will Merges
*/
#ifndef ARP_H
#define ARP_H

#include "common.h"

#define ARP_ETHERTYPE 0x0608 // byte order reversed

#define ETH_HTYPE     0x0100 // byte order reversed
#define IPV4_PTYPE    0x0008 // byte order reversed
#define ETH_HLEN      0x06
#define IPV4_PLEN     0x04
#define ARP_OP_REQ    0x0100 // byte order reversed
#define ARP_OP_REPLY  0x0200 // byte order reversed

// 28 bytes
// https://en.wikipedia.org/wiki/Address_Resolution_Protocol
#pragma pack(1)
typedef struct {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];
    uint32_t spa;
    uint8_t tha[6];
    uint32_t tpa;
} ARP_packet_t;

// respond to an ARP request
// packet is the received ARP packet (encapsulated in ethernet frame)
// len is the length of the packet
// ip is the ip address of our NIC
// no need for a return, don't care if something goes wrong (TODO maybe change this)
void __arp_respond(const uint8_t* data, uint16_t len, uint32_t ip);

#endif
