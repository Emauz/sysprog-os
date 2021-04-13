/*
*   file:  ip.c
*
*   IPv4 header
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


uint8_t __ipv4_add_header(uint8_t* ptr, uint8_t offset, uint16_t len, pid_t pid) {
    // check if big enough

    // make ipv4_hdr obj
    

    // memcpy the ipheader into ptr
    // TODO : call udp layer (add_hdr function)
    // memcpy the data into the next part of the packet

    return 1;
}
