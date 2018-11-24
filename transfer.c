#include "transfer.h"

extern FILE* logFile;

bool tcp_receive(int socket, packet* package){

	byte_t buffer[MAX_BUFFER_SIZE];

	if(package == NULL){
		fprintf(stderr, "%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	size_t bytes_received = recv(socket, buffer, MAX_BUFFER_SIZE, 0);
	if(bytes_received == -1 || bytes_received == 0){
        if( bytes_received == -1 )
		    fprintf(stderr,"%s: recv error received\n", getTime());	
        else
            fprintf(stderr,"%s: recv shutdown received\n", getTime());	
		return FALSE;
	}

	if(!unserialize(buffer, bytes_received, package)){
		return FALSE;	
	}

	return TRUE;
}

bool tcp_send(int socket, packet* package){
	
	byte_t *buffer;

	if(package == NULL){
		fprintf(stderr,"%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	if((buffer = serialize(package)) == NULL) {
		return FALSE;
	}

	size_t bytes_sent = send(socket, buffer, MAX_BUFFER_SIZE, 0);
	if(bytes_sent == -1){
		fprintf(stderr,"%s: send error\n", getTime());	
		return FALSE;
	}

	free_packet(package);
	free(buffer);

	return TRUE;
}


