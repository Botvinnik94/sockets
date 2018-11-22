#include "packet.h"

bool unserialize(byte_t *buffer, size_t buffer_size, packet *package)
{
    int tam;
    char *start_filename;
    char *start_mode;


    if( package == NULL || buffer == NULL)
    {
        //fprintf(logFile, "%s: Invalid buffer or package input\n", getTime());
        return FALSE;
    }

    package->opcode = network_to_host_short(buffer);

    switch(package->opcode)
    {
        case RRQ:
        case WRQ:
            start_filename = buffer + 2;
            start_mode = start_filename + strlen(start_filename) + 1;
            package->request_message.filename = malloc(sizeof(char) * strlen(start_filename));
            package->request_message.mode = malloc(sizeof(char) * strlen(start_mode));
            strcpy(package->request_message.filename, start_filename);
            strcpy(package->request_message.mode, start_mode);
            break;
        case DATA:
            package->data_message.data = malloc(sizeof(byte_t) * (buffer_size - 4));
            package->data_message.nBloq = network_to_host_short(buffer + 2);
            memcpy(package->data_message.data, buffer + 4, buffer_size - 4);
            package->data_message.data_size = buffer_size - 4;
            break;
        case ACK:
            package->data_message.nBloq = network_to_host_short(buffer + 2);
            break;
        case ERR:
            package->err_message.err_code = network_to_host_short(buffer + 2);
            strcpy(package->err_message.msg, buffer + 4);
            break;
        default:
            //fprintf(logFile,"%s: Error reading option code\n",getTime());
            return FALSE;
    }

    return TRUE;
}

bool serialize(packet *package, byte_t *buffer) {
    
}

uint16_t network_to_host_short(byte_t *data)
{
    uint16_t temp;
    memcpy(&temp, data, sizeof(uint16_t));
    return ntohs(temp);
}