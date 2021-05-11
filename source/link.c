/*
*   @file   link.c
*
*   Link layer network header implementation
*
*   @author Sarah Strickman & Will Merges
*/
#include "link.h"
#include "ip.h"
#include "eth.h"
#include "arp.h"
#include "klib.h"

// debug
#include "cio.h"

uint16_t _link_add_header(uint8_t* buff, uint16_t len, msg_t* msg) {
    if(sizeof(LINKhdr_t) + 4 > len) { // header plus frame check sequence
        return 0;
    }

    // setup header
    LINKhdr_t* hdr = (LINKhdr_t*)buff;

    hdr->dst_mac[0] = msg->dst_MAC[0];
    hdr->dst_mac[1] = msg->dst_MAC[1];
    hdr->dst_mac[2] = msg->dst_MAC[2];
    hdr->dst_mac[3] = msg->dst_MAC[3];
    hdr->dst_mac[4] = msg->dst_MAC[4];
    hdr->dst_mac[5] = msg->dst_MAC[5];

    // let the NIC fill in src MAC
    // ACTUALLY this config option is not enabled by default (NSAI bit of config command, page 75 of the manual)
    // so we fill in the src MAC address ourselves
    hdr->src_mac[0] = _eth_MAC[0];
    hdr->src_mac[1] = _eth_MAC[1];
    hdr->src_mac[2] = _eth_MAC[2];
    hdr->src_mac[3] = _eth_MAC[3];
    hdr->src_mac[4] = _eth_MAC[4];
    hdr->src_mac[5] = _eth_MAC[5];

    hdr->ethertype = IPV4_ETHERTYPE;

    uint16_t size = _ipv4_add_header(buff + sizeof(LINKhdr_t), len - sizeof(LINKhdr_t), msg);
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

uint16_t _link_parse_frame(msg_t* msg, uint16_t len, const uint8_t* data) {
    if(sizeof(LINKhdr_t) > len) {
        return 0;
    }

    LINKhdr_t* hdr = (LINKhdr_t*)data;
    msg->dst_MAC[0] = hdr->dst_mac[0];
    msg->dst_MAC[1] = hdr->dst_mac[1];
    msg->dst_MAC[2] = hdr->dst_mac[2];
    msg->dst_MAC[3] = hdr->dst_mac[3];
    msg->dst_MAC[4] = hdr->dst_mac[4];
    msg->dst_MAC[5] = hdr->dst_mac[5];

    msg->src_MAC[0] = hdr->src_mac[0];
    msg->src_MAC[1] = hdr->src_mac[1];
    msg->src_MAC[2] = hdr->src_mac[2];
    msg->src_MAC[3] = hdr->src_mac[3];
    msg->src_MAC[4] = hdr->src_mac[4];
    msg->src_MAC[5] = hdr->src_mac[5];

    if(hdr->ethertype == IPV4_ETHERTYPE) {
        return _ipv4_parse_frame(msg, len - sizeof(LINKhdr_t), data + sizeof(LINKhdr_t));
    } else if(hdr->ethertype == ARP_ETHERTYPE) {
        _arp_respond(data + sizeof(LINKhdr_t), len - sizeof(LINKhdr_t), _ip_addr);
    }
    return 0; // nothing that the user needs, either ARP or unsupported protocol
}
