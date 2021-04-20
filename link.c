/*
*   file:  link.c
*
*   ethernet frame header
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

#ifdef LINK_DEBUG
#include "cio.h"
#include "sio.h"
#endif


uint8_t __link_add_header(uint8_t* data, uint16_t len, pid_t pid) {
    if((len >> 14) != 0 || (len) > 1500) {
        return LINK_TOO_LARGE;
    }
    
    if(len < 14) {
        __cio_printf("LINK alloc fail\n");
        return LINK_NO_MEM;
    }

    uint8_t dataShift = __ipv4_add_header(data + sizeof(LINKhdr_t), len - sizeof(LINKhdr_t), pid);

    // __cio_printf("CBL: %08x", (uint32_t)data);
    __memcpy(data + sizeof(LINKhdr_t) + dataShift, data, ((uint32_t )len) - sizeof(LINKhdr_t));

    // filler data
    uint8_t testDstMac [6] = {0, 0, 0, 0, 0, 0};
    uint8_t testSrcMac [6] = {0, 0, 0, 0, 0, 0};
    uint8_t testType [2] = {0x08, 0x00};

    // setup header
    LINKhdr_t* linkHdr = (LINKhdr_t*)data;
    __memcpy(linkHdr->dest_mac, testDstMac, sizeof(testDstMac));
    __memcpy(linkHdr->src_mac, testSrcMac, sizeof(testSrcMac));
    __memcpy(linkHdr->type, testType, sizeof(testType));

    // pass to lower level
    __eth_tx(data, len, pid);    // set the command to send the packet

    return LINK_SUCCESS;
}


uint8_t __link_set_dest(uint8_t* data, uint16_t len, uint8_t dest[]) {

    // should be 6 bytes long
    if (sizeof(dest) != 6) {
        return LINK_ERR;
    }

    LINKhdr_t* linkHdr = (LINKhdr_t*)data;
    __memcpy(linkHdr->dest_mac, dest, sizeof(dest));

    return LINK_SUCCESS;
}
