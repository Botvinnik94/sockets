#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE 512

/* OPTION DEFINES */
#define EMPTY   0b00000000
#define RRQ     0b00000001
#define WRQ     0b00000010
#define DATA    0b00000011
#define ACK     0b00000100
#define ERR     0b00000101

#define TRUE 1
#define FALSE 0

typedef char bool;

typedef char byte_t;

typedef struct
{
    char *filename;
    char *mode;
}request_msg;

typedef struct
{
    //byte_t nBloq[2];
    uint16_t nBloq;
    byte_t *data;
    size_t data_size;
}data_msg;

typedef struct
{
    uint16_t nBloq;
}ack_msg;

typedef struct
{
    uint16_t err_code;
    char *msg;
}err_msg;

typedef struct
{
    uint16_t opcode;
    
    union
    {
        request_msg request_message;
        data_msg data_message;
        ack_msg ack_message;
        err_msg err_message;
    };
    
}packet;

bool unserialize(byte_t *buffer, size_t buffer_size, packet *package);
bool serialize(packet *package, byte_t *buffer);

uint16_t network_to_host_short(byte_t *data);

#endif

