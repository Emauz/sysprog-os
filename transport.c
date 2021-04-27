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


int __udp_parse_frame(msg_t* msg, uint16_t len, uint8_t* data) {
    if(sizeof(UDPhdr_t) > len) {
        return 0; // too small
    }

    UDPhdr_t* hdr = (UDPhdr_t*)data;

    msg->src_port = hdr->src_port;
    msg->dst_port = hdr->dst_port;

    uint16_t size = hdr->len[1];
    size |= hdr->len[0] << 8;

    len -= sizeof(UDPhdr_t);
    if(len < size) {
        return 0; // something went wrong, we're saying we have more data than we do
    } // if len > size we have some junk on the end of the packet, that's okay for now

    // TODO could check the checksum if there's one

    uint16_t min = size;
    if(msg->len < size) { // need to copy fewer bytes than we actually got
        min = msg->len;
    }

    // copy what we can
    __memcpy(msg->data, data + sizeof(UDPhdr_t), min);

    // fill in the ACTUAL size we got
    // may be different than the buffer size the user specified
    msg->len = size;

    return 1;
}
