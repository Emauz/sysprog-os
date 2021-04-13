/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "eth.h"
#include "ip.h"
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

    // setup cmd
    ipv4hdr_t* IpHdr = (ipv4hdr_t*)data;
    IpHdr->ver_ihl = IPV4_VER_IHL;
    IpHdr->dscp_ecn = 0x00;
    IpHdr->tot_len = len + IPV4_HDR_LEN;
    IpHdr->id = pid;
    IpHdr->flags_offset = IPV4_FLAGS_OFFSET;
    IpHdr->ttl = TTL_DEFAULT;           // filler value for now
    IpHdr->protocol = UDP_PROTOCOL;     // TODO get protocol from later in the packet
    IpHdr->checksum = 0x00;
    IpHdr->src_addr = 0x00;
    IpHdr->dest_addr = 0x00;

    // in simplified mode, the data goes directly after the command block
    __memcpy(IpHdr + 1, data, len);
    

    // make ipv4_hdr obj
    uint8_t ethTx = __eth_tx(data, len, pid);
    

    // memcpy the ipheader into data
    // TODO : call eth layer (add_hdr function)
    // memcpy the data into the next part of the packet

    return 1;
}
