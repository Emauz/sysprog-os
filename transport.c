/*
*   file:  eth.c
*
*   Intel 8255x Ethernet Device Driver header
*/
#include "eth.h"
#include "link.h"
#include "ip.h"
#include "transport.h"
#include "common.h"
#include "kdefs.h"
#include "x86pic.h"
#include "klib.h"
#include "queues.h"

#ifdef TL_DEBUG
#include "cio.h"
#include "sio.h"
#endif


uint8_t __udp_add_header(uint8_t* data, uint16_t len, pid_t pid) {
    if((len >> 14) != 0 || (len - UPD_HDR_LEN) > 1500) {
        return TL_TOO_LARGE;
    }
    
    if(len < UPD_HDR_LEN) {
        __cio_printf("UDP alloc fail\n");
        return TL_NO_MEM;
    }

    // __cio_printf("CBL: %08x", (uint32_t)data);

    //__memcpy(data + sizeof(UDPhdr_t), data, len);

    // setup cmd
    UDPhdr_t* UdpHdr = (UDPhdr_t*)data;
    UdpHdr->src_port = 0x00; 
    UdpHdr->dest_port = 0x00;
    UdpHdr->len = len;       // udp header length + payload length
    UdpHdr->checksum = 0x00;


    // make ipv4_hdr obj
    //return __ipv4_add_header(data, len, pid);
    return sizeof(UDPhdr_t);

}
