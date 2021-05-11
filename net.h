/*
*   @file   net.h
*
*   Networking structures and inline helper functions
*
*   @author Will Merges
*/
#ifndef NET_H
#define NET_H

#include "common.h"

// describes a network message
// all addresses and ports should be in network order
// len can be in system endianness (little endian for x86)
typedef struct {
    uint16_t src_port; // port to send from
    uint16_t dst_port; // port to send to / receive from
    uint32_t src_addr; // ipv4 address to receive FROM
    uint32_t dst_addr; // ipv4 address to send TO
    uint8_t dst_MAC[6]; // destination MAC address, 48-bits, remove if we ever implement ARP requests
    uint8_t src_MAC[6]; // source MAC address, 48-bits
    uint16_t len; // length of 'data' (in system's endianness, not network order)
    uint8_t* data; // data buffer to read/write from
} msg_t;


// set a uint16 to network order
static inline uint16_t hton16(uint16_t num) {
    uint8_t ret[2];
    ret[0] = num >> 8;
    ret[1] = num;
    return *((uint16_t*)ret);
}

// sets address to network order address represented by 'str'
// returns 1 on success, 0 on error
// str is like "192.168.0.1"
static inline int htons(char* str, uint32_t* addr) {
    uint8_t buff[4];

    int i = 0;
    int j = 0;
    int k = 0;
    uint8_t scratch = 0;
    while(i < 15) { // 15 is the max characters in an address (e.g. 255.255.255.255)
        if(str[i] == '\0') {
            buff[j] = scratch;
            break;
        }

        scratch *= 10;
        scratch += ((uint8_t)str[i] - '0');
        k++;

        if(k > 3) {
            // too many digits for 8 bits
            return 0;
        }
        if(str[i + 1] == '.') {
            buff[j] = scratch;
            scratch = 0;
            j++;
            k = 0;
            i++;
        }
        i++;
    }

    if(j != 3) {
        // didn't read a full address
        return 0;
    }

    // addr should be big endian order
    *addr = 0;
    uint8_t* addr_buff = (uint8_t*)addr;
    addr_buff[0] = buff[0];
    addr_buff[1] = buff[1];
    addr_buff[2] = buff[2];
    addr_buff[3] = buff[3];

    return 1;
}

#endif
