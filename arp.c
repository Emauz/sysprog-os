/*
*   @file   arp.c
*
*   ARP protocol implementation
*
*   @author Will Merges
*/
#include "arp.h"
#include "klib.h"
#include "link.h"
#include "eth.h"

// debug
#include "cio.h"

typedef struct {
    LINKhdr_t eth;
    ARP_packet_t arp;
} packet_t;

packet_t _arp_out;

void _arp_respond(const uint8_t* data, uint16_t len, uint32_t ip) {
    if(len < sizeof(ARP_packet_t)) { // can have extra padding on the end
        #ifdef ARP_DEBUG
        __cio_printf("arp size mismatch\n");
        #endif
        return;
    }

    ARP_packet_t* packet = (ARP_packet_t*)data;

    if(packet->htype != ETH_HTYPE) {
        #ifdef ARP_DEBUG
        __cio_printf("arp htype mismatch\n");
        #endif
        return;
    }

    if(packet->ptype != IPV4_PTYPE) {
        #ifdef ARP_DEBUG
        __cio_printf("arp ptype mismatch\n");
        #endif
        return;
    }

    if(packet->hlen != ETH_HLEN) {
        #ifdef ARP_DEBUG
        __cio_printf("arp hlen mismatch\n");
        #endif
        return;
    }

    if(packet->plen != IPV4_PLEN) {
        #ifdef ARP_DEBUG
        __cio_printf("arp plen mismatch\n");
        #endif
        return;
    }

    // only reply to requests
    // TODO do something else if we get a reply?
    if(packet->oper != ARP_OP_REQ) {
        #ifdef ARP_DEBUG
        __cio_printf("arp oper mismatch\n");
        #endif
        return;
    }

    // they're not looking for our ip
    if(packet->tpa != ip) {
        #ifdef ARP_DEBUG
        __cio_printf("arp ip mismatch\n");
        #endif
        return;
    }

    // make our ARP packet reply
    _arp_out.arp.htype = ETH_HTYPE;
    _arp_out.arp.ptype = IPV4_PTYPE;
    _arp_out.arp.hlen = ETH_HLEN;
    _arp_out.arp.plen = IPV4_PLEN;
    _arp_out.arp.oper = ARP_OP_REPLY;
    _arp_out.arp.sha[0] = _eth_MAC >> 40; // our MAC
    _arp_out.arp.sha[1] = _eth_MAC >> 32;
    _arp_out.arp.sha[2] = _eth_MAC >> 24;
    _arp_out.arp.sha[3] = _eth_MAC >> 16;
    _arp_out.arp.sha[4] = _eth_MAC >> 8;
    _arp_out.arp.sha[5] = _eth_MAC;
    _arp_out.arp.spa = ip;
    // copy their MAC
    __memcpy(&_arp_out.arp.tha, &packet->sha, 6);
    _arp_out.arp.tpa = packet->spa;

    // fill in Ethernet header
    _arp_out.eth.ethertype = ARP_ETHERTYPE;
    __memcpy(&_arp_out.eth.dst_mac, &packet->sha, 6);
    _arp_out.eth.src_mac[0] = _eth_MAC >> 40;
    _arp_out.eth.src_mac[1] = _eth_MAC >> 32;
    _arp_out.eth.src_mac[2] = _eth_MAC >> 24;
    _arp_out.eth.src_mac[3] = _eth_MAC >> 16;
    _arp_out.eth.src_mac[4] = _eth_MAC >> 8;
    _arp_out.eth.src_mac[5] = _eth_MAC;

    // send it
    _eth_tx((uint8_t*)&_arp_out, sizeof(ARP_packet_t) + sizeof(LINKhdr_t), 0);
}
