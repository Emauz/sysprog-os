#ifndef SOCKET_TEST_H_
#define SOCKET_TEST_H_

#include "../socket.h"

/**
** Socket test: Write data to socket
*/
int32_t socket_test( uint32_t arg1, uint32_t arg2 ) {
    // announce our presence
    write( CHAN_SIO, "socket_test starting\n", 21 );

    // set up the command-line arguments:
    // userY char 10


    exit( 0 );

    return( 42 );  // shut the compiler up!
}

#endif
