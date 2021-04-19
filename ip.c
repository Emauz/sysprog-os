/*
*   file:  ip.c
*
*   IPv4 header
*/
#include "link.h"
#include "ip.h"
#include "eth.h"
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

    // __cio_printf("\nlen size: %x \n", len);
    // __cio_printf("\nfirst item in data: 0x%x \n", *data & 0xff);


    __memcpy(data + sizeof(NETipv4hdr_t), data, len - sizeof(NETipv4hdr_t));
    __cio_printf("\nfirst item in data: 0x%x \n", *(data + sizeof(NETipv4hdr_t)) & 0xff);


    // setup cmd
    NETipv4hdr_t* IpHdr = (NETipv4hdr_t*)data;

    // __memcpy(IpHdr->ver_ihl, ipv3_ver_ihl, sizeof(IpHdr->ver_ihl));
    // __memcpy(IpHdr->dscp_ecn, 0, sizeof(IpHdr->dscp_ecn));
    // __memcpy(IpHdr->tot_len, len, sizeof(IpHdr->tot_len));
    // __memcpy(IpHdr->id, (uint16_t) pid, sizeof(IpHdr->id));
    // __memcpy(IpHdr->flags_offset, ipv4_flags_offset, sizeof(IpHdr->flags_offset));
    // __memcpy(IpHdr->ttl, ttl_default, sizeof(IpHdr->ttl));
    // __memcpy(IpHdr->protocol, udp_protocol, sizeof(IpHdr->protocol));
    // __memcpy(IpHdr->checksum, 0, sizeof(IpHdr->checksum));
    // __memcpy(IpHdr->src_addr, 0, sizeof(IpHdr->src_addr));
    // __memcpy(IpHdr->dest_addr, 0x7F000001, sizeof(IpHdr->dest_addr));   // hardcoded for testing

    IpHdr->ver_ihl = IPV4_VER_IHL;
    IpHdr->dscp_ecn = 0x00;
    IpHdr->tot_len = len;
    IpHdr->id = pid;
    IpHdr->flags_offset = 0x4000;
    IpHdr->ttl = 0x40;           // filler value for now
    IpHdr->protocol = 0x11;      // TODO get protocol from later in the packet
    IpHdr->checksum = 0x00;
    IpHdr->src_addr = 0x00000000;
    IpHdr->dest_addr = 0x7F000001;      // 127.0.0.1 for testing    

    __cio_printf("\nip header: %x \n", IpHdr);

    // return __eth_tx(data, len, pid);     // for testing

    // make link_hdr obj
    __cio_printf("\nfirst item in link header: 0x%x \n", *data & 0xff);
    return __link_add_header(data, len, pid);
    
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
