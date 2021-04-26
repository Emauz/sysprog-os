/*
*   file:  socket.c
*
*   Socket module implementations
*/
#include "eth.h"
#include "ip.h"
#include "socket.h"
#include "support.h"
#include "common.h"
#include "kdefs.h"
#include "x86pic.h"
#include "klib.h"
#include "queues.h"
#include "scheduler.h"
#include "link.h"
#include "net.h"

#ifdef IP_DEBUG
#include "cio.h"
#include "sio.h"
#endif


/*
** PRIVATE GLOBAL VARIABLES
*/
static queue_t _cmd_process_q = NULL;

/*
** PUBLIC FUNCTIONS
*/

// /*
// ** Blocking call to send ethernet frame over the network
// ** Process will be blocked until transmit has completed, after which
// ** it will be added back to the ready queue
// **
// ** @param sender sending process' PCB
// ** @param data   data to include in ethernet frame
// ** @param len    length of data to be sent
// */
// void _socket_send_frame( uint8_t* data, uint32_t len ) {
//     // get pid from sender's PCB
//     pid_t sender_pid = _current->pid;
//
//     // queue up frame to be sent
//     uint8_t tx_status = __eth_tx( data, len, sender_pid );
//     assert( tx_status == ETH_SUCCESS );
//
//     // sleep the sending process
//     _current->state = Sleeping;
//
//     // Add sender's PCB to the sending queue (block until sent)
//     int enque_status = _que_enque( _tx_process_q, _current, 0 );
//     assert( enque_status == E_SUCCESS );
// }

// big transmit buffer
#define TX_BUFF_SIZE 2048
uint8_t tx_buffer[TX_BUFF_SIZE];

/**
**  _socket_send - write to the network
**
**  implements: netsend(msg_t* msg)
**      sends a message over the network
**/
void _socket_send(msg_t* msg) {
    // create the packet
    uint16_t size = __link_add_header(tx_buffer, TX_BUFF_SIZE, msg);
    assert( size != 0 );

    // queue up frame to be sent
    if(ETH_SUCCESS != __eth_tx(tx_buffer, size, _current->pid)) {
        RET(_current) = SOCKET_ERR;
    }

    // sleep the sending process
    _current->state = Sleeping;

    // Add sender's PCB to the command queue (block until complete)
    assert(E_SUCCESS == _que_enque( _cmd_process_q, _current, 0 ));
}

// TODO documentation
// TODO recv callback function
void _socket_recv(msg_t* msg) {
    // add msg to a linked list holding the PCB of the caller as well
    // that's it

    // callback needs to search the list after parsing part of the packet
}

/**
**  _socket_setip - set the IP of the system
**
**  implements: setip(uint32_t addr)
**/
void _socket_setip(uint32_t addr) {
    _ip_addr = addr;
    RET(_current) = SOCKET_SUCCESS;
}

/**
**  _socket_setMAC - set the MAC address of the system
**
**  implements: setMAC(uint64_t addr)
**/
void _socket_setMAC(uint8_t addr[6]) {
    uint64_t mac = (uint64_t)addr[0] << 40;
    mac |= (uint64_t)addr[1] << 32;
    mac |= (uint64_t)addr[2] << 24;
    mac |= (uint64_t)addr[3] << 16;
    mac |= (uint64_t)addr[4] << 8;
    mac |= (uint64_t)addr[5];

    if(ETH_SUCCESS != __eth_loadaddr(mac, _current->pid)) {
        RET(_current) = SOCKET_ERR;
    }

    _current->state = Sleeping;

    assert(E_SUCCESS == _que_enque(_cmd_process_q, _current, 0));
}


/*
** Callback for when networking device completes a command (TX/loadaddr) job
**
** Wakes the process that was waiting on that job to finish
*/
void _socket_cmd_cb( uint16_t id, uint16_t status ) {
    if(id == 0) { // kernel id, do nothing
        return;
    }

    // dequeue first process from command queue
    pcb_t *proc = _que_deque( _cmd_process_q );
    assert( proc != NULL );

    // ensure that the sender we dequeued has the same PID we were passed
    assert( id == proc->pid );

    // set the return
    // should be ETH_CMD_FAIL or ETH_CMD_SUCCESS
    if(status != ETH_SUCCESS) {
        RET(proc) = SOCKET_ERR;
    } else {
        RET(proc) = SOCKET_SUCCESS;
    }

    // schedule process to be run again
    _schedule( proc );
}

/*
** Initialize socket module
*/
void _socket_init( void ) {
    // initialie queue for transmitting processes
    _cmd_process_q = _que_alloc( NULL );

    // set ethernet callback
    __eth_set_cmd_callback( _socket_cmd_cb );
}
