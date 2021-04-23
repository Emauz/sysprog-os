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

#ifdef IP_DEBUG
#include "cio.h"
#include "sio.h"
#endif


/*
** PRIVATE GLOBAL VARIABLES
*/
static queue_t _tx_process_q = NULL;

/*
** PUBLIC FUNCTIONS
*/

/*
** Blocking call to send ethernet frame over the network
** Process will be blocked until transmit has completed, after which
** it will be added back to the ready queue
**
** @param sender sending process' PCB
** @param data   data to include in ethernet frame
** @param len    length of data to be sent
*/
void __socket_send_frame( uint8_t* data, uint32_t len ) {
    // get pid from sender's PCB
    pid_t sender_pid = _current->pid;

    // queue up frame to be sent
    uint8_t tx_status = __eth_tx( data, len, sender_pid );
    assert( tx_status == ETH_SUCCESS );

    // sleep the sending process
    _current->state = Sleeping;

    // Add sender's PCB to the sending queue (block until sent)
    int enque_status = _que_enque( _tx_process_q, _current, 0 );
    assert( enque_status == E_SUCCESS );

}

/*
** Callback for when networking device completes TX/RX job
** 
** Wakes the process that was waiting on that job to finish
**
** TODO: Implement RX callback 
*/
void __socket_cb( uint16_t id, uint16_t status ) {
    // dequeue first process from sending queue
    pcb_t *sender = _que_deque( _tx_process_q );
    assert( sender != NULL );

    // ensure that the sender we dequeued has the same PID we were passed
    assert( id == sender->pid );

    // schedule sender process to be ran again
    _schedule( sender );
}

/*
** Initialize socket module
*/
void __socket_init( void ) {
    // initialie queue for transmitting processes
    _tx_process_q = _que_alloc( NULL );

    // set ethernet callback
    __eth_set_cmd_callback( __socket_cb );
}

