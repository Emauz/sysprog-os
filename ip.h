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
#define IP_ERR 1 // general error, can make more specific (see eth.h for reference)
#define PTR_tOO_SMALL

// header lengths
#define IPV4_HDR_LEN 20
#define IPV6_HDR_LEN 288    // unused

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
} ipv4_hdr;


// adds an IPv4 header to an ethernet frame.
// ptr:     the pointer to add the header
// offset:  how many bytes to shift the ipv4 header (where to start writing)
// len:     total length of the pointer
// pid:     unused, process id
uint8_t __ipv4_add_header(uint8_t* ptr, uint8_t offset, uint16_t len, pid_t pid);

#endif
