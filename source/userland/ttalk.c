/*
*   @file   ttalk.c
*
*   TigerTalk chat server
*
*   Listens on OUR_IP:TTALK_PORT for UDP packets and allows the user to respond
*   over serial I/O
*
*   @author Eric Moss & Will Merges
*/
#ifndef TTALK_H_
#define TTALK_H_

#include "../sysnet.h"
#include "../net.h"
#include "../eth.h"
#include "../cio.h"

#define DATA_BUFFER_SIZE 256

#define OUR_IP "10.0.2.15"
#define TTALK_PORT 8081

/*
 * creates a message struct with given data and length, then sends
 *
 * msg: Data to be sent to target
 * len: Length of data provided
 */
void send_msg(char *msg, int32_t len, uint8_t dst_MAC[6], uint32_t dst_addr, uint16_t dst_port) {
    // hard code the message to be sent
    msg_t message;
    message.src_port = hton16(TTALK_PORT);
    message.dst_port = dst_port;
    // message.dst_port = hton16(TTALK_PORT);
    message.dst_addr = dst_addr;
    message.dst_MAC[0] = dst_MAC[0];
    message.dst_MAC[1] = dst_MAC[1];
    message.dst_MAC[2] = dst_MAC[2];
    message.dst_MAC[3] = dst_MAC[3];
    message.dst_MAC[4] = dst_MAC[4];
    message.dst_MAC[5] = dst_MAC[5];
    message.len = len;
    message.data = (uint8_t*)msg;

    // send message over the network
    netsend(&message);
}

/*
 * Recieves a packet and prints out the payload
 *
 * Copies source MAC, IP, and port out
 */
void recv_and_print( uint8_t mac[6], uint32_t* their_ip, uint16_t* their_port ) {
    // construct message structure to recieve into
    uint8_t data_buffer[DATA_BUFFER_SIZE];
    msg_t message;
    message.dst_port = hton16(TTALK_PORT);
    message.len = DATA_BUFFER_SIZE;
    message.data = data_buffer;
    // receive message into this struct
    write( CHAN_SIO, "--> ", 5 );
    int status = netrecv(&message);
    if(status != SOCKET_SUCCESS) {
        write(CHAN_SIO, "recv error in TigerTalk\r\n", 25);
        return;
    }
    // write( CHAN_SIO, "TigerTalk recieved message!\r\n", 29 );

    // ding the bell!
    char ding = 0x07;
    write( CHAN_CONS, &ding, 1);

    // copy the src MAC out
    mac[0] = message.src_MAC[0];
    mac[1] = message.src_MAC[1];
    mac[2] = message.src_MAC[2];
    mac[3] = message.src_MAC[3];
    mac[4] = message.src_MAC[4];
    mac[5] = message.src_MAC[5];

    // copy src port out
    *their_port = message.src_port;

    // copy src addr out
    *their_ip = message.src_addr;

    // determine how much we should print out
    uint16_t data_recieved = DATA_BUFFER_SIZE;
    if(message.len < DATA_BUFFER_SIZE) {
        data_recieved = message.len;
    }
    // echo to console I/O
    write( CHAN_SIO, &data_buffer, data_recieved );
    write( CHAN_SIO, "\r\n> ", 4 );
}

/**
** TigerTalk: demonstrate OS networking capabilities with simple chat client
**
** Client sends message to our IP (defined above), and we respond with whatever
** response is entered on console I/O
*/
int32_t ttalk( uint32_t arg1, uint32_t arg2 ) {
    // announce our presence
    write( CHAN_SIO, "==== TigerTalk ====\r\n", 21 );

    // set our MAC
    uint8_t mac[6] = {0x00, 0x00, 0xDE, 0xAD, 0xBE, 0xEF}; // d3adb33f
    setMAC(mac);

    // set our host IP
    uint32_t source_addr;
    htons(OUR_IP, &source_addr);
    setip(source_addr);

    // enter test loop of writing what comes in over SIO
    char buf[DATA_BUFFER_SIZE] = {0};
    char message[DATA_BUFFER_SIZE] = {0};
    int bytes_read;
    int message_bytes = 0;
    uint8_t their_mac[6];
    uint16_t their_port;
    uint32_t their_ip;
    recv_and_print(their_mac, &their_ip, &their_port);
    while(1) {
        // receive message and print to console I/O

        // read input from user
        bytes_read = read( CHAN_SIO, &buf, DATA_BUFFER_SIZE );
        buf[bytes_read] = NULL;
        write( CHAN_SIO, &buf, DATA_BUFFER_SIZE );
        // if user types 'enter', send message over network
        if(buf[0] == '\n' || buf[0] == '\r') {
            // write( CHAN_CONS, "\r\nTigerTalk sending message\r\n", 29 );
            write( CHAN_CONS, message, message_bytes);
            send_msg(message, message_bytes, their_mac, their_ip, their_port);
            // reset message buffer
            message[0] = NULL;
            message_bytes = 0;
            // recieve next input from other side
            recv_and_print(their_mac, &their_ip, &their_port);
        }
        // append read character to full message
        else {
            strcat(message, buf);
            message_bytes++;
        }
    }

    exit( 0 );

    return( 42 );  // shut the compiler up!
}

#endif
