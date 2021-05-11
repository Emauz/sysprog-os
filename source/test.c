/*
*   @file   test.c
*
*   Various system test function definitions that should be called before
*   any processes start
*
*   @author Will Merges & Sarah Strickman
*/
#include "pci.h"
#include "cio.h"
#include "common.h"
#include "link.h"
#include "ip.h"
#include "transport.h"
#include "eth.h"
#include "net.h"
#include "arp.h"
#include "klib.h"
#include "test.h"

// placeholder receive callback function
// responds to ARP requests and prints IPv4/UDP packets to Console I/O
void rx_callback(uint16_t status,  const uint8_t* data, uint16_t count) {
    __cio_printf("count: %04x\n", count);
    msg_t temp;
    uint8_t buff[2048];
    temp.data = buff;
    temp.len = 2048;
    int ret = _link_parse_frame(&temp, count, data);
    __cio_printf("ret: %02x\n", ret);
    if(!ret) {
        __cio_printf("return early rx cb\n");
        return;
    }

    __cio_printf("len: %04x\n", temp.len);
    for(unsigned int i = 0; i < temp.len; i++) {
        __cio_printf("%c", temp.data[i]);
    }
    __cio_printf("\n");
}

// basic test of adding Ethernet, IPv4, and UDP headers to a packet and transmitting it
// also passes the basic receive callback function to the ethernet driver
void __packet_test(void) {
    htons("10.0.2.15", &_ip_addr);
    _eth_set_rx_callback(&rx_callback);

    uint8_t buff[2000];
    msg_t msg;
    msg.src_port = 1;
    msg.dst_port = 2;
    msg.dst_addr = 0x0;
    msg.dst_MAC[0] = 0xFF;
    msg.dst_MAC[1] = 0xFF;
    msg.dst_MAC[2] = 0xFF;
    msg.dst_MAC[3] = 0xFF;
    msg.dst_MAC[4] = 0xFF;
    msg.dst_MAC[5] = 0xFF;
    msg.len = 4;
    msg.data = (uint8_t*)"test"; // somewhere in RO data

    // fills in src port, src MAC for us
    uint16_t size = _link_add_header(buff, 2000, &msg);

    for(uint16_t i = 0; i < size; i++) {
        __cio_printf("%02x ", buff[i]);
        if(i % 4 == 0) {
            __cio_printf("\n");
        }
    }

    _eth_tx(buff, size, 0);
}

// brute force scan the entire PCI bus and print out the vendor ID, device ID,
// and interrupt line of every device found
void __pci_test(void) {
  uint16_t vendorID;
  uint16_t deviceID;
  uint8_t int_line;
  for(int bus = 0; bus < 256; bus++) {
    for(int slot = 0; slot < 32; slot++) {
        vendorID = __pci_read16(bus, slot, 0, PCI_VENDOR_ID_OFFSET);
        deviceID = __pci_read16(bus, slot, 0, PCI_DEVICE_ID_OFFSET);
        int_line = __pci_read8(bus, slot, 0, PCI_INT_LINE_OFFSET);

        if(vendorID != 0xFFFF) {
          __cio_printf("0x%04x -- 0x%04x -- %02x\n", vendorID, deviceID, int_line);
        }
      }
    }
}
