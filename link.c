/*
*   file:  link.c
*
*   ethernet frame header
*/
#include "eth.h"
#include "link.h"
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
    if((len >> 14) != 0 || (len - LINK_HDR_LEN) > 1500) {
        return LINK_TOO_LARGE;
    }
    
    if(len < 20) {
        __cio_printf("LINK alloc fail\n");
        return LINK_NO_MEM;
    }

    __cio_printf("CBL: %08x", (uint32_t)data);
    __memcpy(data + sizeof(LINKhdr_t) + 1, data, len - sizeof(LINKhdr_t));

    uint8_t testDstMac [6] = {0x00, 0x12, 0x34, 0x56, 0x78, 0x90};
    uint8_t testSrcMac [6] = {0, 0, 0, 0, 0, 0};
    uint8_t testType [2] = {0x08, 0x00};

    // setup cmd
    LINKhdr_t* linkHdr = (LINKhdr_t*)data;
    __memcpy(linkHdr->dest_mac, testDstMac, sizeof(testDstMac));
    __memcpy(linkHdr->src_mac, testSrcMac, sizeof(testSrcMac));
    __memcpy(linkHdr->type, testType, sizeof(testType));

    // in simplified mode, the data goes directly after the command block
    
    

    // make ipv4_hdr obj
    uint8_t ethTx = __eth_tx(data, len, pid);

    return ethTx;
}
