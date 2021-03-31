#include "pci.h"
#include "cio.h"
#include "common.h"

void __pci_test() {
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
