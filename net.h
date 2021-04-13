#ifndef NET_H
#define NET_H

#include "common.h"

// sets address to network order address represented by 'str'
// returns 1 on success, 0 on error
static inline int htons(char* str, uint32_t* addr) {
    uint8_t buff[4];

    int i = 0;
    int j = 0;
    int k = 0;
    uint8_t scratch = 0;
    while(i < 15) { // 15 is the max characters in an address (e.g. 255.255.255.255)
        if(str[i] == '\0') {
            buff[j] = scratch - ('0' * k);
            break;
        }

        scratch += (uint8_t)str[i];
        k++;

        if(k > 3) {
            // too many digits for 8 bits
            return 0;
        }
        if(str[i + 1] == '.') {
            buff[j] = scratch - ('0' * k);
            scratch = 0;
            j++;
            k = 0;
            i++;
        }
        i++;
    }

    if(j != 3) {
        // didn't read a full address
        return 0;
    }

    // reverse order (little endian to big endian)
    *addr = 0;
    uint8_t* addr_buff = (uint8_t*)addr;
    addr_buff[0] = buff[0];
    addr_buff[1] = buff[1];
    addr_buff[2] = buff[2];
    addr_buff[3] = buff[3];

    return 1;
}

#endif
