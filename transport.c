/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "transport.h"
#include "common.h"
#include "klib.h"

#ifdef ETH_DEBUG
#include "cio.h"
#include "sio.h"
#endif


uint16_t __udp_add_header(uint8_t* buff, uint16_t len, msg_t* msg) {
    if((sizeof(UDPhdr_t) + msg->len) > len) {
        return 0;
    }

    UDPhdr_t* hdr = (UDPhdr_t*)buff;

    __cio_printf("UDP header: %x\n", hdr);

    hdr->src_port = msg->src_port;
    hdr->dst_port = msg->dst_port;
    hdr->len[0] = (sizeof(UDPhdr_t) + msg->len) >> 8;
    hdr->len[1] = (sizeof(UDPhdr_t) + msg->len);
    hdr->checksum = 0x0; // UDP checksum is optional in ipv4 but mandatory in ipv6

    __memcpy(buff + sizeof(UDPhdr_t), msg->data, msg->len);

    return sizeof(UDPhdr_t) + msg->len;
}


int __udp_parse_frame(msg_t* msg, uint16_t len, const uint8_t* data) {
    if(sizeof(UDPhdr_t) > len) {
        return 0; // too small
    }
    __cio_printf("udp parse\n");

    UDPhdr_t* hdr = (UDPhdr_t*)data;

    // set ports
    msg->src_port = hdr->src_port;
    msg->dst_port = hdr->dst_port;

    // set msg pointer
    msg->data = data + sizeof(UDPhdr_t);

    uint16_t size = hdr->len[1];
    size |= (hdr->len[0] << 8);
    size -= sizeof(UDPhdr_t);

    __cio_printf("UDP size: %04x, len: %04x\n", size, len);
    if(size > len - sizeof(UDPhdr_t)) {
        __cio_printf("udp err\n");
        return 0; // packet is larger than what we stored, size should equal len minus the header size
    }

    msg->len = size;

    // TODO could check the checksum if there's one

    return 1;
}
