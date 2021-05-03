/*
*   file:  link.c
*
*   ethernet frame header
*/
#include "link.h"
#include "ip.h"
#include "eth.h"
#include "arp.h"
#include "klib.h"

// debug
#include "cio.h"

uint16_t __link_add_header(uint8_t* buff, uint16_t len, msg_t* msg) {
    if(sizeof(LINKhdr_t) + 4 > len) { // header plus frame check sequence
        return 0;
    }

    // setup header
    LINKhdr_t* hdr = (LINKhdr_t*)buff;

    hdr->dst_mac[0] = msg->dst_MAC >> 40;
    hdr->dst_mac[1] = msg->dst_MAC >> 32;
    hdr->dst_mac[2] = msg->dst_MAC >> 24;
    hdr->dst_mac[3] = msg->dst_MAC >> 16;
    hdr->dst_mac[4] = msg->dst_MAC >> 8;
    hdr->dst_mac[5] = msg->dst_MAC;

    // let the NIC fill in src MAC
    // ACTUALLY this config option is not enabled by default (NSAI bit of config command, page 75 of the manual)
    // so we fill in the src MAC address ourselves
    hdr->src_mac[0] = _eth_MAC >> 40;
    hdr->src_mac[1] = _eth_MAC >> 32;
    hdr->src_mac[2] = _eth_MAC >> 24;
    hdr->src_mac[3] = _eth_MAC >> 16;
    hdr->src_mac[4] = _eth_MAC >> 8;
    hdr->src_mac[5] = _eth_MAC;

    hdr->ethertype = IPV4_ETHERTYPE;

    uint16_t size = __ipv4_add_header(buff + sizeof(LINKhdr_t), len - sizeof(LINKhdr_t), msg);
    if(size == 0) {
        return 0;
    }

    size += sizeof(LINKhdr_t);

    // pad the payload if it's too small
    while(size < ETH_PAYLOAD_MIN_SIZE + sizeof(LINKhdr_t)) {
        buff[size++] = 0x0;
    }

    // zero the frame check sequence, ACTUALLY it seems like the NIC tacks this on at the end automagically
    // __memset(buff + size + sizeof(LINKhdr_t), 4, 0); // the NIC will fill in the CRC for us
    // return size + 4;

    return size;
}

int __link_parse_frame(msg_t* msg, uint16_t len, const uint8_t* data) {
    if(sizeof(LINKhdr_t) > len) {
        return 0;
    }

    LINKhdr_t* hdr = (LINKhdr_t*)data;
    msg->dst_MAC = (uint64_t)hdr->dst_mac[0] << 40;
    msg->dst_MAC |= (uint64_t)hdr->dst_mac[1] << 32;
    msg->dst_MAC |= (uint64_t)hdr->dst_mac[2] << 24;
    msg->dst_MAC |= (uint64_t)hdr->dst_mac[3] << 16;
    msg->dst_MAC |= (uint64_t)hdr->dst_mac[4] << 8;
    msg->dst_MAC |= (uint64_t)hdr->dst_mac[5];

    msg->src_MAC = (uint64_t)hdr->src_mac[0] << 40;
    msg->src_MAC |= (uint64_t)hdr->src_mac[1] << 32;
    msg->src_MAC |= (uint64_t)hdr->src_mac[2] << 24;
    msg->src_MAC |= (uint64_t)hdr->src_mac[3] << 16;
    msg->src_MAC |= (uint64_t)hdr->src_mac[4] << 8;
    msg->src_MAC |= (uint64_t)hdr->src_mac[5];

    if(hdr->ethertype == IPV4_ETHERTYPE) {
        return __ipv4_parse_frame(msg, len - sizeof(LINKhdr_t), data + sizeof(LINKhdr_t));
    } else if(hdr->ethertype == ARP_ETHERTYPE) {
        __arp_respond(data + sizeof(LINKhdr_t), len - sizeof(LINKhdr_t), _ip_addr);
    }
    return 0; // nothing that the user needs, either ARP or unsupported protocol
}
