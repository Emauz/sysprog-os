#ifndef TTALK_H_
#define TTALK_H_

#include "../socket.h"
#include "../net.h"
#include "../eth.h"
#include "../cio.h"

#define DATA_BUFFER_SIZE 256

#define OUR_IP "10.0.2.15"
#define THEIR_IP "10.0.0.0"
#define TTALK_PORT 79

/*
 * creates a message struct with given data and length, then sends
 *
 * msg: Data to be sent to target
 * len: Length of data provided
 */
void send_msg( char *msg, int32_t len ) {
    // hard code the message to be sent
    msg_t message;
    message.src_port = hton16(TTALK_PORT);
    message.dst_port = hton16(TTALK_PORT);
    htons(THEIR_IP, &message.dst_addr); // TODO: Test sending to a real IP
    message.dst_MAC = 0x000068f728688cec;
    message.len = len;
    message.data = (uint8_t*)msg;

    // send message over the network
    netsend(&message);
}

void recv_and_print( void ) {
    // construct message structure to recieve into
    uint8_t data_buffer[DATA_BUFFER_SIZE];
    msg_t message;
    message.dst_port = TTALK_PORT; 
    htons(OUR_IP, &message.src_addr);
    message.len = DATA_BUFFER_SIZE;
    message.data = data_buffer;
    // recieve message into this struct
    netrecv(&message);

    // determine how much we should print out
    uint16_t data_recieved = DATA_BUFFER_SIZE;
    if(message.len < DATA_BUFFER_SIZE) {
        data_recieved = message.len;
    }
    // echo to console I/O
    write( CHAN_SIO, &data_buffer, data_recieved );
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
    write( CHAN_SIO, "tigertalk waiting to recieve message\r\n", 38 );

    // set our host IP
    uint32_t source_addr;
    htons(OUR_IP, &source_addr);
    setip(source_addr);

    // test recieve and print to console I/O
    //recv_and_print();
    
    

    // enter test loop of writing what comes in over SIO
    char buf[DATA_BUFFER_SIZE] = {0};
    char message[DATA_BUFFER_SIZE] = {0};
    int bytes_read;
    int message_bytes = 0;
    write( CHAN_SIO, "TigerTalk entering test echo loop\n\r", 35 );
    while(1) {
        bytes_read = read( CHAN_SIO, &buf, DATA_BUFFER_SIZE );
        buf[bytes_read] = NULL;
        write( CHAN_SIO, &buf, DATA_BUFFER_SIZE );
        // if user types 'enter', send message over network
        if(buf[0] == '\n' || buf[0] == '\r') {
            write( CHAN_CONS, "\nTigerTalk sending message\n", 27 );
            write( CHAN_CONS, message, message_bytes);
            send_msg(message, message_bytes);
            // reset message buffer
            message[0] = NULL;
            message_bytes = 0;
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
