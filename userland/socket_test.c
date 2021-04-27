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


    // hard code the message to be sent
    msg_t message;
    message.src_port = 25565;
    message.dst_port = 25565; 
    htons("1.2.3.4", &message.dst_addr); // TODO: Test sending to a real IP
    message.dst_MAC = 0xdeadbeef; // TODO: change once ARP is working
    message.len = 23;
    message.data = (uint8_t*)"socket_test: hello world!";

    // send message over the network
    netsend(&message);
        
    // alert that we've completed our syscall
    write( CHAN_SIO, "socket_test completed netsend syscall\n", 38 );

    exit( 0 );

    return( 42 );  // shut the compiler up!
}

#endif
