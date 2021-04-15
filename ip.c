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


uint8_t __ipv4_add_header(uint8_t* data, uint16_t len, pid_t pid) {
    if((len >> 14) != 0 || (len - IPV4_HDR_LEN) > 1500) {
        return IP_TOO_LARGE;
    }
    
    if(len < 20) {
        __cio_printf("IP alloc fail\n");
        return IP_NO_MEM;
    }

    __cio_printf("CBL: %08x", (uint32_t)data);


    __memcpy(data + sizeof(NETipv4hdr_t), data, len - sizeof(NETipv4hdr_t));

    // values
    uint8_t testVer_Ihl [1] = {0x45};
    uint8_t testZero [1] = {0};

    // setup cmd
    NETipv4hdr_t* IpHdr = (NETipv4hdr_t*)data;
    __memcpy(IpHdr->ver_ihl, testVer_Ihl, sizeof(testVer_Ihl));
    __memcpy(IpHdr->dscp_ecn, testZero, sizeof(testZero));
    __memcpy(IpHdr->tot_len, len, sizeof(IpHdr->tot_len));
    __memcpy(IpHdr->id, (uint16_t) pid, sizeof(IpHdr->id));
    __memcpy(IpHdr->flags_offset, 0x4000, sizeof(IpHdr->flags_offset));
    __memcpy(IpHdr->ttl, 0x40, sizeof(IpHdr->ttl));
    __memcpy(IpHdr->protocol, 0x11, sizeof(IpHdr->protocol));
    __memcpy(IpHdr->checksum, 0, sizeof(IpHdr->checksum));
    __memcpy(IpHdr->src_addr, 0, sizeof(IpHdr->src_addr));
    __memcpy(IpHdr->dest_addr, 0x7F000001, sizeof(IpHdr->dest_addr));

    // IpHdr->ver_ihl = IPV4_VER_IHL;
    // IpHdr->dscp_ecn = 0x00;
    // IpHdr->tot_len = len;
    // IpHdr->id = pid;
    // IpHdr->flags_offset = 0x4000;
    // IpHdr->ttl = 0x40;           // filler value for now
    // IpHdr->protocol = 0x11;      // TODO get protocol from later in the packet
    // IpHdr->checksum = 0x00;
    // IpHdr->src_addr = 0x00;
    // IpHdr->dest_addr = 0x7F000001;      // 127.0.0.1 for testing    

    // make ipv4_hdr obj
    uint8_t linkHdr = __link_add_header(data, len, pid);
    

    return linkHdr;
}
