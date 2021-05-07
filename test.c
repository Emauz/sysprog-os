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

// test callback for now
// only responds to ARPs
void rx_callback(uint16_t status,  const uint8_t* data, uint16_t count) {
    __cio_printf("count: %04x\n", count);
    msg_t temp;
    uint8_t buff[2048];
    temp.data = buff;
    temp.len = 2048;
    int ret = __link_parse_frame(&temp, count, data);
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

void __packet_test(void) {
    htons("10.0.2.15", &_ip_addr);
    _eth_set_rx_callback(&rx_callback);

    uint8_t buff[2000];
    msg_t msg;
    msg.src_port = 1;
    msg.dst_port = 2;
    msg.dst_addr = 0x0;
    msg.dst_MAC = 0x0;
    msg.len = 4;
    msg.data = (uint8_t*)"test"; // somewhere in RO data

    // fills in src port, src MAC for us
    uint16_t size = __link_add_header(buff, 2000, &msg);

    // for(uint16_t i = 0; i < size; i++) {
    //     __cio_printf("%02x ", buff[i]);
    //     if(i % 4 == 0) {
    //         __cio_printf("\n");
    //     }
    // }

    _eth_tx(buff, size, 0);
}

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


void __link_test(void) {
  uint8_t arr[5 + 14];
  char test[6];

  __memcpy(arr, "test", sizeof(test));
  __link_add_header((uint8_t*) arr, sizeof(arr), 0);
  // __link_add_header((uint8_t*)"test2", 18, 1);
  // __link_add_header((uint8_t*)"ip test2", 19, 2);
  // __link_add_header((uint8_t*)"ip test2", 19, 3);

  // uint8_t arr[1 + 4 + 14];
  // __memcpy(arr, "test", 5);
  // __link_add_header((uint8_t*) arr, 4 + 14, 0);
  // __link_add_header((uint8_t*)"test", 128, 0);
  // __link_add_header((uint8_t*)"test2", 18, 1);
  // __link_add_header((uint8_t*)"ip test2", 19, 2);
  // __link_add_header((uint8_t*)"ip test2", 19, 3);
}

void __ip_test(void) {
  // __ipv4_add_header((uint8_t*)"ip test", 42, 0);
  // __ipv4_add_header((uint8_t*)"ip test2", 42, 1);
  // __ipv4_add_header((uint8_t*)"ip test2", 43, 2);
  // __ipv4_add_header((uint8_t*)"ip test2", 43, 3);
}

void __transport_test(void) {
  // __udp_add_header((uint8_t*)"ip test", 24, 0);
  // __udp_add_header((uint8_t*)"ip test2", 24, 1);
  // __udp_add_header((uint8_t*)"ip test2", 25, 2);
  // __udp_add_header((uint8_t*)"ip test2", 25, 3);
}
