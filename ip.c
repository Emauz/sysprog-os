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
    // uint16_t ip_eth_hdrlen = sizeof(TxActionCmd_t) + len + IPV4_HDR_LEN;
    // if() {
    //     return ETH_TOO_LARGE;
    // }

    return 1;
}
