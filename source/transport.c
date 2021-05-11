/*
*   @file   transport.h
*
*   Transport layer network header (UDP) implementation
*
*   @author Sarah Strickman & Will Merges
*/
#include "transport.h"
#include "common.h"
#include "klib.h"

#ifdef ETH_DEBUG
#include "cio.h"
#include "sio.h"
#endif

// calculates the UDP checksum of msg
// https://gist.github.com/fxlv/81209bbd150abfeaceb1f85ff076c9f3
// udp_hdr.checksum must be zeroed before calling
uint16_t _udp_checksum(const msg_t* msg, UDPhdr_t* udp_hdr) {
    uint32_t sum = 0;

    // add payload
    for(int i = 0; i < msg->len; i++) {
        if(i & 1) { // odd, low byte
            sum += (uint32_t)msg->data[i];
        } else { // even, high byte
            sum += (uint32_t)msg->data[i] << 8;
        }
    }

    uint8_t* addrs = (uint8_t*)&(msg->src_addr);
    for(int i = 0; i < 8; i++) {
        if(i & 1) { // odd, low byte
            sum += (uint32_t)addrs[i];
        } else { // even, high byte
            sum += (uint32_t)addrs[i] << 8;
        }
    }

    uint8_t* data = (uint8_t*)udp_hdr;
    for(int i = 0; i < sizeof(UDPhdr_t); i++) {
        if(i & 1) { // odd, low byte
            sum += (uint32_t)data[i];
        } else { // even, high byte
            sum += (uint32_t)data[i] << 8;
        }
    }

    sum += UDP_PROTOCOL;
    sum += msg->len + sizeof(UDPhdr_t);

    while(sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return hton16(~sum);
}

uint16_t _udp_add_header(uint8_t* buff, uint16_t len, msg_t* msg) {
    if((sizeof(UDPhdr_t) + msg->len) > len) {
        return 0;
    }

    UDPhdr_t* hdr = (UDPhdr_t*)buff;

    hdr->src_port = msg->src_port;
    hdr->dst_port = msg->dst_port;
    hdr->len[0] = (sizeof(UDPhdr_t) + msg->len) >> 8;
    hdr->len[1] = (sizeof(UDPhdr_t) + msg->len);
    hdr->checksum = 0x0; // UDP checksum is optional in ipv4, but Linux drops them
    hdr->checksum = _udp_checksum(msg, hdr);

    __memcpy(buff + sizeof(UDPhdr_t), msg->data, msg->len);

    return sizeof(UDPhdr_t) + msg->len;
}


uint16_t _udp_parse_frame(msg_t* msg, uint16_t len, const uint8_t* data) {
    if(sizeof(UDPhdr_t) > len) {
        return 0; // too small
    }

    UDPhdr_t* hdr = (UDPhdr_t*)data;

    // set ports
    msg->src_port = hdr->src_port;
    msg->dst_port = hdr->dst_port;

    // set msg pointer
    msg->data = data + sizeof(UDPhdr_t);

    uint16_t size = hdr->len[1];
    size |= (hdr->len[0] << 8);
    size -= sizeof(UDPhdr_t);

    if(size > len - sizeof(UDPhdr_t)) {
        return 0; // packet is larger than what we stored, size should equal len minus the header size
    }

    msg->len = size;

    // TODO could check the checksum if there's one

    return 1;
}
