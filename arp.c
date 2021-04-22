/*
*   ARP protocol
*/
#include "arp.h"
#include "klib.h"
#include "eth.h"

ARP_packet_t _arp_out;

void __arp_respond(uint8_t* data, uint16_t len, uint32_t ip) {
    if(len != sizeof(ARP_packet_t)) {
        return;
    }

    ARP_packet_t* packet = (ARP_packet_t*)data;

    if(packet->htype != ETH_HTYPE) {
        return;
    }

    if(packet->ptype != IPV4_PTYPE) {
        return;
    }

    if(packet->hlen != ETH_HLEN) {
        return;
    }

    if(packet->plen != IPV4_PLEN) {
        return;
    }

    // only reply to requests
    // TODO do something else if we get a reply?
    if(packet->oper != ARP_OP_REQ) {
        return;
    }

    // they're not looking for our ip
    if(packet->tpa != ip) {
        return;
    }

    // make our reply
    _arp_out.htype = ETH_HTYPE;
    _arp_out.ptype = IPV4_PTYPE;
    _arp_out.hlen = ETH_HLEN;
    _arp_out.plen = IPV4_PLEN;
    _arp_out.oper = ARP_OP_REPLY;
    _arp_out.sha[0] = _eth_MAC >> 40; // our MAC
    _arp_out.sha[1] = _eth_MAC >> 32;
    _arp_out.sha[2] = _eth_MAC >> 24;
    _arp_out.sha[3] = _eth_MAC >> 16;
    _arp_out.sha[4] = _eth_MAC >> 8;
    _arp_out.sha[5] = _eth_MAC;
    _arp_out.spa = ip;
    // copy their MAC
    __memcpy(&_arp_out.tha, &packet->spa, 6);
    _arp_out.tpa = packet->spa;

    // send it
    __eth_tx((uint8_t*)&_arp_out, sizeof(ARP_packet_t), 0);
}
