/*
*   @file  sysnet.h
*
*   Network syscalls module declarations
*
*   @author Eric Moss and Will Merges
*/
#ifndef SOCKET_H
#define SOCKET_H

#include "common.h"
#include "process.h"
#include "net.h"

// return values
#define SOCKET_SUCCESS E_SUCCESS
#define SOCKET_ERR E_FAILURE

// Initialize the socket module
void _sysnet_init( void );

/*
** Blocking call to send message over the network
** Process will be blocked until transmit has completed, after which
** it will be added back to the ready queue
**
** @param msg the message to send
*/
void _sysnet_send( msg_t* msg );

/*
** Blocking call to receive message over the network
**
** @param msg   the structure to place the received message in
*/
void _sysnet_recv( msg_t* msg );

/*
** Set the IP address of the system
**
** @param addr the IP address to set
*/
void _sysnet_setip( uint32_t addr );

/*
** Set the MAC address of the system
**
** @param addr the 48-bit network order MAC address to set
*/
void _sysnet_setMAC( uint8_t addr[6] );


#endif
