/*
*   file:  link.c
*
*   ethernet frame header
*/
#include "link.h"
#include "ip.h"
#include "klib.h"

// debug
#include "cio.h"

uint16_t __link_add_header(uint8_t* buff, uint16_t len, msg_t* msg) {
    if(sizeof(LINKhdr_t) + 4 > len) { // header plus frame check sequence
        return 0;
    }

    // setup header
    LINKhdr_t* hdr = (LINKhdr_t*)buff;
    __cio_printf("LINK HEADER: %x\n", hdr);

    hdr->dst_mac[0] = msg->dst_MAC >> 40;
    hdr->dst_mac[1] = msg->dst_MAC >> 32;
    hdr->dst_mac[2] = msg->dst_MAC >> 24;
    hdr->dst_mac[3] = msg->dst_MAC >> 16;
    hdr->dst_mac[4] = msg->dst_MAC >> 8;
    hdr->dst_mac[5] = msg->dst_MAC;

    // let the NIC fill in src MAC
    hdr->src_mac[0] = 0x0;
    hdr->src_mac[1] = 0x0;
    hdr->src_mac[2] = 0x0;
    hdr->src_mac[3] = 0x0;
    hdr->src_mac[4] = 0x0;
    hdr->src_mac[5] = 0x0;

    hdr->ethertype = IPV4_ETHERTYPE;

    uint16_t size = __ipv4_add_header(buff + sizeof(LINKhdr_t), len - sizeof(LINKhdr_t), msg);
    if(size == 0) {
        return 0;
    }

    // TODO pad the payload to at least 48 bytes

    // zero the frame check sequence
    __memset(buff + size + sizeof(LINKhdr_t), 4, 0); // the NIC will fill in the CRC for us

    return size + sizeof(LINKhdr_t) + 4;
}
