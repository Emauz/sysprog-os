/*
*   file:  socket.h
*
*   Socket module declarations
*/
#ifndef SOCKET_H
#define SOCKET_H

#include "common.h"
#include "process.h"

// return values
#define SOCKET_SUCCESS 0
#define SOCKET_ERR 1 

// Initialize the socket module
void __socket_init( void );

/*
** Blocking call to send ethernet frame over the network
** Process will be blocked until transmit has completed, after which
** it will be added back to the ready queue
**
** @param data   data to include in ethernet frame
** @param len    length of data to be sent
*/
void __socket_send_frame( uint8_t* data, uint32_t len );

#endif
