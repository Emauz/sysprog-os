#ifndef SOCKET_TEST_H_
#define SOCKET_TEST_H_

#include "../socket.h"
#include "../net.h"
#include "../eth.h"
#include "../cio.h"

/**
** Socket test: Write data to socket
*/
int32_t socket_test( uint32_t arg1, uint32_t arg2 ) {
    // announce our presence
    write( CHAN_SIO, "socket_test starting\n", 21 );

    uint32_t ip;
    htons("10.0.2.15", &ip);
    setip(ip);

    // hard code the message to be sent
    msg_t message;
    message.src_port = 25565;
    message.dst_port = 25565;
    htons("10.10.10.2", &message.dst_addr);
    message.dst_MAC = 0x0;
    message.len = 23;
    message.data = (uint8_t*)"socket_test: hello world!";

    // send message over the network
    for(int i = 0; i < 5; i++) {
        netsend(&message);
    }

    // alert that we've completed our syscall
    write( CHAN_SIO, "socket_test completed netsend syscall\n", 38 );

    // try a receive on port 8080
    write(CHAN_SIO, "receiving on port 8081\n", 23);

    message.dst_port = hton16(8081);
    message.len = 100;
    uint8_t temp_data[100];
    message.data = temp_data;
    int ret = netrecv(&message);
    if(ret != SOCKET_SUCCESS) {
        write(CHAN_SIO, "recv error!\n", 12);
    } else {
        write(CHAN_SIO, "received: ", 10);
        for(int i = 0; i < message.len && i < 100; i++) {
            write(CHAN_SIO, &message.data[i], 1);
        }
        write(CHAN_SIO, "\n", 1);
    }

    exit( 0 );

    return( 42 );  // shut the compiler up!
}

#endif
