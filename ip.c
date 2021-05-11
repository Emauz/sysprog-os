/*
*   @file   ip.c
*
*   IPv4 network header implementation
*
*   @author Sarah Strickman & Will Merges
*/
#include "eth.h"
#include "link.h"
#include "ip.h"
#include "transport.h"
#include "support.h"
#include "common.h"
#include "kdefs.h"
#include "x86pic.h"
#include "klib.h"
#include "queues.h"

// debug
#include "cio.h"

uint32_t _ip_addr = 0x0; // initialize to 0

uint16_t _ipv4_checksum(const uint16_t* data, uint16_t len) {
    uint16_t sum = 0;
    for(int i = 0; i < (len / sizeof(uint16_t)); i++) {
        sum += data[i];
    }

    // retrun one's complement
    return ~sum;
}

uint16_t _ipv4_add_header(uint8_t* buff, uint16_t len, msg_t* msg) {
    if(sizeof(NETipv4hdr_t) > len) {
        return 0;
    }

    NETipv4hdr_t* hdr = (NETipv4hdr_t*)buff;

    hdr->ver_ihl = IPV4_VER_IHL;
    hdr->dscp_ecn = 0x00;
    hdr->id = 0;
    hdr->ttl = TTL_DEFAULT;
    hdr->flags_offset = IPV4_FLAGS_OFFSET;
    hdr->protocol = UDP_PROTOCOL; // if we want to support multiple transport layers, check msg for this value
    hdr->src_addr = _ip_addr;
    msg->src_addr = _ip_addr; // fill in message struct for the user
    hdr->dst_addr = msg->dst_addr;

    uint16_t size = _udp_add_header(buff + sizeof(NETipv4hdr_t), len - sizeof(NETipv4hdr_t), msg);
    if(size == 0) {
        return 0;
    }

    hdr->tot_len[0] = (sizeof(NETipv4hdr_t) + size) >> 8;
    hdr->tot_len[1] = sizeof(NETipv4hdr_t) + size;

    uint16_t checksum = _ipv4_checksum((uint16_t*)hdr, sizeof(NETipv4hdr_t));
    hdr->checksum[0] = checksum;
    hdr->checksum[1] = checksum >> 8;

    return size + sizeof(NETipv4hdr_t);
}


uint16_t _ipv4_parse_frame(msg_t* msg, uint16_t len, const uint8_t* data) {
    if(sizeof(NETipv4hdr_t) > len) {
        return 0; // error, small packet
    }


    __cio_printf("ip\n");

    // TODO could do more error checking, but oh well
    NETipv4hdr_t* hdr = (NETipv4hdr_t*)data;

    if(hdr->dst_addr != _ip_addr) {
        return 0; // packet wasn't meant for us, oopsies
    }

    msg->dst_addr = hdr->dst_addr;
    msg->src_addr = hdr->src_addr;

    if(hdr->protocol == UDP_PROTOCOL) {
        return _udp_parse_frame(msg, len - sizeof(NETipv4hdr_t), data + sizeof(NETipv4hdr_t));
    }

    return 0; // unsupported protocol
}
