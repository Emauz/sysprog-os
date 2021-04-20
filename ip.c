/*
*   file:  ip.c
*
*   IPv4 header
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

#ifdef IP_DEBUG
#include "cio.h"
#include "sio.h"
#endif


// header values
uint8_t ipv3_ver_ihl = 0x45;            // version 4, header length 20
uint16_t ipv4_flags_offset = 0x4000;    // do not fragment bit set, fragment = 0x00
uint8_t ttl_default = 0x40;             // 64 in decimal
uint8_t udp_protocol = 0x11;            // 17 in decimal


uint8_t __ipv4_add_header(uint8_t* data, uint16_t len, pid_t pid) {
    if((len >> 14) != 0 || (len) > 1500) {
        return IP_TOO_LARGE;
    }
    
    if(len < 20) {
        __cio_printf("IP alloc fail\n");
        return IP_NO_MEM;
    }

    uint8_t hilen = len << 8;
    uint8_t lolen = len;
    uint16_t revlen = ((uint16_t) lolen << 8) | hilen;

    __cio_printf("\nip len: %x \n", len);
    __cio_printf("\nip len (reversed): %x \n", revlen);
    // __cio_printf("\nfirst item in data: 0x%x \n", *data & 0xff);

    uint8_t dataShift = __udp_add_header(data + sizeof(NETipv4hdr_t), len - (uint16_t) sizeof(NETipv4hdr_t), pid);

    if (dataShift == 0) {
        return 0;
    }

    // __cio_printf("CBL: %08x", (uint32_t)data);
    //__memcpy(data + sizeof(NETipv4hdr_t) + dataShift, data, ((uint32_t )len) - sizeof(LINKhdr_t));

    // __cio_printf("\nfirst item in data: 0x%x \n", *(data + sizeof(NETipv4hdr_t)) & 0xff);

    // setup cmd
    NETipv4hdr_t* IpHdr = (NETipv4hdr_t*)data;

    IpHdr->ver_ihl = IPV4_VER_IHL;
    IpHdr->dscp_ecn = 0x00;
    IpHdr->tot_len = revlen;
    IpHdr->id = pid;
    IpHdr->flags_offset = IPV4_FLAGS_OFFSET;
    IpHdr->ttl = 0x40;           // filler value for now
    IpHdr->protocol = 0x11;      // TODO get protocol from later in the packet
    IpHdr->checksum = 0x00;
    IpHdr->src_addr = 0x00000000;
    IpHdr->dest_addr = 0x7F000001;      // 127.0.0.1 for testing    

    __cio_printf("\nip header: %x \n", IpHdr);

    // return __eth_tx(data, len, pid);     // for testing

    // make link_hdr obj
    return (sizeof(NETipv4hdr_t) + dataShift);
    
}


uint8_t __ip_set_dest(uint8_t* data, uint16_t len, uint8_t dest[]) {

    // should be 4 bytes long
    if (sizeof(dest) != 4) {
        return LINK_ERR;
    }

    NETipv4hdr_t* linkHdr = (NETipv4hdr_t*)data;
    __memcpy(linkHdr->dest_addr, dest, sizeof(dest));

    return LINK_SUCCESS;
}
