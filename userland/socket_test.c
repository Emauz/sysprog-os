/*
*   @file   Socket test process
*
*   Tests various network syscalls
*
*   @author Eric Moss
*
*/
#ifndef SOCKET_TEST_H_
#define SOCKET_TEST_H_

#include "../sysnet.h"
#include "../net.h"
#include "../eth.h"
#include "../cio.h"

/**
** Socket test: Write data to socket
*/
int32_t socket_test( uint32_t arg1, uint32_t arg2 ) {
    // announce our presence
    write( CHAN_SIO, "socket_test starting\r\n", 22 );

    uint8_t mac[6] = {0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a};
    if(setMAC(mac) == E_FAILURE) {
        write(CHAN_SIO, "set MAC failed\r\n", 15);
    }

    uint32_t ip;
    htons("10.0.2.15", &ip);
    setip(ip);

    // hard code the message to be sent
    msg_t message;

    message.src_port = hton16(9001);
    message.dst_port = hton16(9000);
    htons("10.10.10.2", &message.dst_addr);
    message.dst_MAC[0] = 0xFF;
    message.dst_MAC[1] = 0xFF;
    message.dst_MAC[2] = 0xFF;
    message.dst_MAC[3] = 0xFF;
    message.dst_MAC[4] = 0xFF;
    message.dst_MAC[5] = 0xFF;
    message.len = 25;
    message.data = (uint8_t*)"socket_test: hello world!";

    // send message over the network
    for(int i = 0; i < 55; i++) {
        if(E_FAILURE == netsend(&message)) {
            write(CHAN_SIO, "tx failed\r\n", 11);
        }
    }

    // alert that we've completed our syscall
    write(CHAN_SIO, "socket_test completed netsend syscall\r\n", 37);

    // try a receive on port 8080
    write(CHAN_SIO, "receiving on port 8081\r\n", 24);

    message.dst_port = hton16(8081);
    message.len = 100;
    uint8_t temp_data[100];
    message.data = temp_data;
    int ret = netrecv(&message);
    if(ret != SOCKET_SUCCESS) {
        write(CHAN_SIO, "recv error!\r\n", 13);
    } else {
        write(CHAN_SIO, "received: ", 10);
        for(int i = 0; i < message.len && i < 100; i++) {
            write(CHAN_SIO, &message.data[i], 1);
        }
        write(CHAN_SIO, "\r\n", 2);
    }

    exit( 0 );

    return( 42 );  // shut the compiler up!
}

#endif
