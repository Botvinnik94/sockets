#include "packet.h"

extern FILE* logFile;

bool unserialize(byte_t *buffer, size_t buffer_size, packet *package)
{
    char *start_filename;
    char *start_mode;


    if( package == NULL || buffer == NULL)
    {
        fprintf(logFile, "%s: Invalid buffer or package input\n", getTime());
        return FALSE;
    }

    package->opcode = network_to_host_short(buffer);

    switch(package->opcode)
    {
        case RRQ:
        case WRQ:
            start_filename = buffer + 2;
            start_mode = start_filename + strlen(start_filename) + 1;
            package->request_message.filename = malloc(sizeof(char) * (strlen(start_filename) + 1));
            package->request_message.mode = malloc(sizeof(char) * (strlen(start_mode) + 1));
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
			package->err_message.msg = malloc(sizeof(char) * (strlen(buffer + 4) + 1));
            strcpy(package->err_message.msg, buffer + 4);
            break;
        default:
            fprintf(logFile,"%s: Error reading option code\n",getTime());
            return FALSE;
    }

    return TRUE;
}

byte_t* serialize(packet *package, size_t *buffer_size) {

    byte_t *buffer;
	uint16_t opcode, temp;

	if( package == NULL ){
		fprintf(logFile, "%s: Invalid buffer or package input\n", getTime());
		return NULL;	
	}

	opcode = htons(package->opcode);

	switch(package->opcode){
		case RRQ:
		case WRQ:
            *buffer_size = sizeof(byte_t) * (strlen(package->request_message.filename) + strlen(package->request_message.mode) + 4);
			buffer = malloc(*buffer_size);
			memcpy(buffer, &opcode, 2);
			strcpy(buffer + 2, package->request_message.filename);
			strcpy(buffer + 2 + strlen(package->request_message.filename) + 1, package->request_message.mode);
			break;
		case DATA:
            *buffer_size = sizeof(byte_t) * (4 + package->data_message.data_size);
			buffer = malloc(*buffer_size);
			memcpy(buffer, &opcode, 2);
			temp = htons(package->data_message.nBloq);
			memcpy(buffer + 2, &temp, 2);
			memcpy(buffer + 4, package->data_message.data, package->data_message.data_size);            
            break;
		case ACK:
            *buffer_size = sizeof(byte_t) * 4;
			buffer = malloc(*buffer_size);
			memcpy(buffer, &opcode, 2);
			temp = htons(package->ack_message.nBloq);
			memcpy(buffer + 2, &temp, 2);
			break;
		case ERR:
            *buffer_size = sizeof(byte_t) * (strlen(package->err_message.msg) + 1 + 4);
			buffer = malloc(*buffer_size);
			memcpy(buffer, &opcode, 2);
			temp = htons(package->err_message.err_code);
			memcpy(buffer + 2, &temp, 2);
			strcpy(buffer + 4, package->err_message.msg);
			break;
		default:
            fprintf(logFile,"%s: Error reading option code\n",getTime());
            return NULL;
	}

	return buffer;
}

void free_packet(packet * package){
	if(package != NULL){
		switch(package->opcode){
			case RRQ:
		    case WRQ:
				free(package->request_message.filename);
		        free(package->request_message.mode);
		        break;
		    case DATA:
		        free(package->data_message.data);
		        break;
			case ERR:
				free(package->err_message.msg);
				break;
		}
	}
}

bool build_RQ_packet(uint16_t type, char* filename, packet *package)
{
	if(package != NULL){
		package->opcode = type;
		package->request_message.filename = malloc(sizeof(char) * (strlen(filename) + 1));
		strcpy(package->request_message.filename, filename);
		package->request_message.mode = malloc(sizeof(char) * (strlen("octet") + 1));
		strcpy(package->request_message.mode, "octet");
		return TRUE;
	}

	return FALSE;
}

bool build_DATA_packet(byte_t *data, size_t data_size, uint16_t nBloq, packet *package)
{
	if(package != NULL){
		package->opcode = DATA;
		package->data_message.nBloq = nBloq;
		package->data_message.data = malloc(sizeof(byte_t) * data_size);
		memcpy(package->data_message.data, data, data_size);
        package->data_message.data_size = data_size;
		return TRUE;
	}

	return FALSE;
}

bool build_ERR_packet(uint16_t type, char *msg, packet *package)
{
    if(package != NULL){
		package->opcode = ERR;
		package->err_message.err_code = type;
		package->err_message.msg = malloc(sizeof(char) * (strlen(msg) + 1));
		strcpy(package->err_message.msg, msg);
		return TRUE;
	}

	return FALSE;
}

bool build_ACK_packet(uint16_t nBloq, packet *package)
{
    if(package != NULL){
		package->opcode = ACK;
		package->ack_message.nBloq = nBloq;
		return TRUE;
	}

	return FALSE;
}

uint16_t network_to_host_short(byte_t *data)
{
    uint16_t temp;
    memcpy(&temp, data, sizeof(uint16_t));
    return ntohs(temp);
}

char * getTime() {
	long timevar;
	time(&timevar);
	char *date = (char *) ctime(&timevar);
	date[strlen(date)-1] = '\0';
	return date; 
}
