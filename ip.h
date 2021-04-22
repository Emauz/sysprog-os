/*
*   file:  ip.h
*
*   IPv4 header construction
*/
#ifndef IP_H
#define IP_H

#include "common.h"

// return values
#define IP_SUCCESS 0
#define IP_ERR 1        // general error, can make more specific (see eth.h for reference)
#define IP_TOO_LARGE 2
#define IP_NO_MEM 3

// header lengths
#define IPV4_HDR_LEN 20    // 20 bytes

// header values
#define IPV4_VER_IHL    0x45        // version 4, header length 20
#define IPV4_FLAGS_OFFSET   0x0040  // do not fragment bit set, fragment = 0x00.  This is 0x4000, but bytes are reversed.
#define TTL_DEFAULT     0x40        // 64 hops before dropped
#define UDP_PROTOCOL    0x11        // 17 in decimal

// IPv4 structure reference: https://www.tutorialspoint.com/ipv4/ipv4_packet_structure.htm
// 20 bytes
typedef struct {
    uint8_t ver_ihl;
    uint8_t dscp_ecn;
    uint8_t tot_len[2];
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint8_t checksum[2];
    uint32_t src_addr;
    uint32_t dst_addr;
} NETipv4hdr_t;

// adds an ipv4 frame to a buffer of length len.
// 'msg' is the message to be sent
// return how large the packet is or 0 on error
// also adds higher layer encapsulated layers
uint16_t __ipv4_add_header(uint8_t* buff, uint16_t len, msg_t* msg);


#endif
