#include "pci.h"
#include "cio.h"
#include "common.h"
#include "link.h"
#include "ip.h"
#include "transport.h"
#include "eth.h"
#include "test.h"

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
  uint8_t arr[128];
  __memcpy(arr, "test", 5);
  __link_add_header((uint8_t*) "test", 128, 0);
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