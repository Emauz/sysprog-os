/*
*   ARP protocol
*/
#include "common.h"

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
