/*
*   file:  link.h
*
*   Link Layer construction
*/
#ifndef LINK_H
#define LINK_H

#include "common.h"

// return values
#define LINK_SUCCESS 0
#define LINK_ERR 1        // general error, can make more specific (see eth.h for reference)
#define LINK_TOO_LARGE 2
#define LINK_NO_MEM 3

// header lengths
#define LINK_HDR_LEN 14 // 14 bytes

// header values
#define IPV4_ETHERTYPE 0x0800

// IPv4 structure reference: https://www.tutorialspoint.com/ipv4/ipv4_packet_structure.htm
typedef struct {
    uint8_t dest_mac  [6];
    uint8_t src_mac [6];
    uint8_t type [2];
} LINKhdr_t;


// adds an ethernet frame to a packet.
// data:    payload. This should be a complete transport layer packet (i.e. UPD packet)
// len:     length of the total packet
// pid:     for syscall items
uint8_t __link_add_header(uint8_t* data, uint16_t len, pid_t pid);



#endif
