#ifndef TTALK_H_
#define TTALK_H_

#include "../sysnet.h"
#include "../net.h"
#include "../eth.h"
#include "../cio.h"

#define DATA_BUFFER_SIZE 256

#define OUR_IP "10.0.2.15"
#define THEIR_IP "10.0.0.0"
#define TTALK_PORT 8086

/*
 * creates a message struct with given data and length, then sends
 *
 * msg: Data to be sent to target
 * len: Length of data provided
 */
void send_msg( char *msg, int32_t len, uint64_t dst_MAC) {
    // hard code the message to be sent
    msg_t message;
    message.src_port = hton16(TTALK_PORT);
    message.dst_port = hton16(TTALK_PORT);
    htons(THEIR_IP, &message.dst_addr); // TODO: Test sending to a real IP
    //message.dst_MAC = 0x000068f728688cec; // Eric's laptop
    message.dst_MAC = dst_MAC; // Eric's laptop
    message.len = len;
    message.data = (uint8_t*)msg;

    // send message over the network
    netsend(&message);
}

/*
 * Recieves a packet and prints out the payload
 *
 * Returns MAC address message was sent from
 */
uint64_t recv_and_print( void ) {
    // construct message structure to recieve into
    uint8_t data_buffer[DATA_BUFFER_SIZE];
    msg_t message;
    message.dst_port = hton16(TTALK_PORT);
    message.len = DATA_BUFFER_SIZE;
    message.data = data_buffer;
    // recieve message into this struct
    write( CHAN_SIO, "TigerTalk waiting to recieve message\r\n", 38 );
    int status = netrecv(&message);
    if(status != SOCKET_SUCCESS) {
        write(CHAN_SIO, "recv error in TigerTalk\r\n", 25);
        return 0;
    }
    write( CHAN_SIO, "TigerTalk recieved message!\r\n", 29 );

    // determine how much we should print out
    uint16_t data_recieved = DATA_BUFFER_SIZE;
    if(message.len < DATA_BUFFER_SIZE) {
        data_recieved = message.len;
    }
    // echo to console I/O
    write( CHAN_SIO, &data_buffer, data_recieved );
    write( CHAN_SIO, "\r\n>", 3 );
    return message.src_MAC;
}

/**
** TigerTalk: demonstrate OS networking capabilities with simple chat client
**
** Client sends message to our IP (defined above), and we respond with whatever
** response is entered on console I/O
*/
int32_t ttalk( uint32_t arg1, uint32_t arg2 ) {
    // announce our presence
    //write( CHAN_SIO, "TigerTalk waiting to recieve message\r\n", 38 );

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
    uint64_t dst_MAC = recv_and_print();
    while(1) {
        // recieve message and print to console I/O

        // read input from user
        bytes_read = read( CHAN_SIO, &buf, DATA_BUFFER_SIZE );
        buf[bytes_read] = NULL;
        write( CHAN_SIO, &buf, DATA_BUFFER_SIZE );
        // if user types 'enter', send message over network
        if(buf[0] == '\n' || buf[0] == '\r') {
            write( CHAN_CONS, "\r\nTigerTalk sending message\r\n", 29 );
            write( CHAN_CONS, message, message_bytes);
            // ding a bell once we've written message!
            write( CHAN_CONS, "\a", 1);
            send_msg(message, message_bytes, dst_MAC);
            // reset message buffer
            message[0] = NULL;
            message_bytes = 0;
            // recieve next input from other side
            dst_MAC = recv_and_print();
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
