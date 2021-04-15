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
#define IPV4_HDR_LEN 160    // 20 bytes

// header values
#define IPV4_VER_IHL    0x45        // version 4, header length 20
#define IPV4_FLAGS_OFFSET   0x4000  // do not fragment bit set, fragment = 0x00
#define TTL_DEFAULT     0x40        // 64 in decimal
#define UDP_PROTOCOL    0x11        // 17 in decimal

// IPv4 structure reference: https://www.tutorialspoint.com/ipv4/ipv4_packet_structure.htm
typedef struct {
    uint8_t ver_ihl;
    uint8_t dscp_ecn;
    uint16_t tot_len;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_addr;
    uint32_t dest_addr;
} NETipv4hdr_t;


// adds an IPv4 header to an ethernet frame.
// data:    payload. This should be a complete transport layer packet (i.e. UPD packet)
// len:     length of the total packet
// pid:     for syscall items
uint8_t* __ip_add_header(uint8_t* data, uint16_t len, pid_t pid);



#endif
