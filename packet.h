#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>

#define MAX_DATA_SIZE 512

/* OPTION DEFINES */
#define EMPTY   0x00
#define RRQ     0x01
#define WRQ     0x02
#define DATA    0x03
#define ACK     0x04
#define ERR     0x05

#define TRUE    1
#define FALSE   0

#define ERR_UNDEFINED       0x00
#define ERR_FILE_NOT_FOUND  0x01
#define ERR_FULL_DISK       0x03
#define ERR_ILLEGAL_OP      0x04
#define ERR_FILE_EXISTS     0x06

typedef char bool;

typedef char byte_t;

typedef struct
{
    char *filename;
    char *mode;
}request_msg;

typedef struct
{
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
byte_t* serialize(packet *package, size_t *buffer_size);

void free_packet(packet * package);
uint16_t network_to_host_short(byte_t *data);

bool build_RQ_packet(uint16_t type, char* filename, packet *package);
bool build_DATA_packet(byte_t *data, size_t data_size, uint16_t nBloq, packet *package);
bool build_ERR_packet(uint16_t type, char *msg, packet *package);
bool build_ACK_packet(uint16_t nBloq, packet *package);

char * getTime();

#endif

