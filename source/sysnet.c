/*
*   @file  sysnet.c
*
*   Network syscalls module implementations
*
*   @author Eric Moss and Will Merges
*/
#include "eth.h"
#include "ip.h"
#include "sysnet.h"
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

// big transmit buffer
#define TX_BUFF_SIZE 2048
static uint8_t _tx_buffer[TX_BUFF_SIZE];

/**
**  _sysnet_send - send over ethernet
**
**  implements: netsend(msg_t* msg)
**      sends a message over ethernet
**/
void _sysnet_send(msg_t* msg) {
    // create the packet
    uint16_t size = _link_add_header(_tx_buffer, TX_BUFF_SIZE, msg);

    if(size == 0) {
        RET(_current) = SOCKET_ERR;
    }

    // queue up frame to be sent
    if(ETH_SUCCESS != _eth_tx(_tx_buffer, size, _current->pid)) {
        RET(_current) = SOCKET_ERR;
    }

    // sleep the sending process
    _current->state = Blocked;

    // Add sender's PCB to the command queue (block until complete)
    assert(E_SUCCESS == _que_enque( _cmd_process_q, _current, 0 ));

    // current process has been put to sleep, schedule a new one
    _dispatch();

    return;
}

// number of processes that can be waiting on a receive at the same time
#define NUM_RECV_PROCESSES 50

typedef struct {
    pcb_t* proc;
    msg_t* msg;
} recv_node_t;

// list of receiving procs and their messages to fill
static recv_node_t _recv_list[NUM_RECV_PROCESSES];

// indicates which indices are free in 'recv_list'
static uint8_t _recv_free_map[NUM_RECV_PROCESSES];

/**
**  _sysnet_recv - receive from ethernet
**
**  implements: netrec(msg_t* msg)
**      receives a message over ethernet
**/
void _sysnet_recv(msg_t* msg) {
    // NOTE: if the system supported memory protection we'd probably want to check that 'data' is set correctly
    for(int i = 0; i < NUM_RECV_PROCESSES; i++) {
        if(!_recv_free_map[i]) { // 0 indicates free, 1 indicates taken
            _recv_free_map[i] = 1;
            _recv_list[i].msg = msg;
            _recv_list[i].proc = _current;

            // sleep the calling proc until we get a message for it
            _current->state = Blocked;

            _dispatch();
            return;
        }
    }

    // no free memory to store this proc, oops
    RET(_current) = SOCKET_ERR;
}

// msg to parse packets into
static msg_t _temp_msg;

// receive callback
// set as ethernet receive callback in _sysnet_init
// parses a received packet, if it's an ARP request to our address, it will reply.
// If it's a Ethernet/IPv4/UDP packet addressed to our system with a dst_port a
// process is waiting for it will copy the data out and wake that process.
// NOTE: will wake up ALL processes waiting for a packet to that dst_port
void _sysnet_recv_cb(uint16_t status, const uint8_t* data, uint16_t count) {
    if(status == ETH_RECV_ERR) { // TODO maybe check the packet too to alert a user there was a failure
        return;
    }

    if(!_link_parse_frame(&_temp_msg, count, data)) {
        return; // bad frame or an ARP, nothing else to do
    } // else we need to pass it to a user

    recv_node_t* node;
    for(int i = 0; i < NUM_RECV_PROCESSES; i++) {
        if(_recv_free_map[i]) { // we found a waiting process
            node = &_recv_list[i];

            // check if someone needs this packet
            // both ports and source address need to match what user said
            // everything else is filled in
            if(node->msg->dst_port != _temp_msg.dst_port) {
                // __cio_printf("ports don't match!\n");
                // __cio_printf("recv: %04x, got: %04x\n", node->msg->dst_port, _temp_msg.dst_port);
                continue;
            }

            // we found a packet someone needs!
            node->msg->src_port = _temp_msg.src_port;
            node->msg->dst_port = _temp_msg.dst_port;
            node->msg->src_addr = _temp_msg.src_addr;
            node->msg->dst_addr = _temp_msg.dst_addr;
            __memcpy(node->msg->dst_MAC, _temp_msg.dst_MAC, 6);
            __memcpy(node->msg->src_MAC, _temp_msg.src_MAC, 6);

            // we have room for the whole payload
            if(node->msg->len >= _temp_msg.len) {
                __memcpy(node->msg->data, _temp_msg.data, _temp_msg.len);
            } else { // we can only store part of it
                __memcpy(node->msg->data, _temp_msg.data, node->msg->len);
            }

            node->msg->len = _temp_msg.len; // len should be however big the packet we got, regardless if we can store it

            // free the memory in the waiting list
            _recv_free_map[i] = 0;

            // successful receive
            RET(node->proc) = SOCKET_SUCCESS;

            // wake up the waiting process
            _schedule(node->proc);
        }
    }
}

/**
**  _sysnet_setip - set the IP of the system
**
**  implements: setip(uint32_t addr)
**/
void _sysnet_setip(uint32_t addr) {
    _ip_addr = addr;
    RET(_current) = SOCKET_SUCCESS;
}

/**
**  _sysnet_setMAC - set the MAC address of the system
**
**  implements: setMAC(uint64_t addr)
**/
void _sysnet_setMAC(uint8_t addr[6]) {
    // uint64_t mac = (uint64_t)(addr[0]) << 40;
    // mac |= (uint64_t)addr[1] << 32;
    // mac |= (uint64_t)addr[2] << 24;
    // mac |= (uint64_t)addr[3] << 16;
    // mac |= (uint64_t)addr[4] << 8;
    // mac |= (uint64_t)addr[5];

    if(ETH_SUCCESS != _eth_loadaddr(addr, _current->pid)) {
        RET(_current) = SOCKET_ERR;
    }

    _current->state = Blocked;

    assert(E_SUCCESS == _que_enque(_cmd_process_q, _current, 0));

    _dispatch();
}


/*
** Callback for when networking device completes a command (TX/loadaddr) job
**
** Wakes the process that was waiting on that job to finish
**
** NOTE: commands executed with an id of 0 means the kernel started the command
**       so the callback should ignore it
*/
void _sysnet_cmd_cb( uint16_t id, uint16_t status ) {
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
void _sysnet_init( void ) {
    // initialie queue for transmitting processes
    _cmd_process_q = _que_alloc( NULL );

    // set NIC callbacks
    _eth_set_cmd_callback( _sysnet_cmd_cb );
    _eth_set_rx_callback( _sysnet_recv_cb );
}
