#include "transfer.h"

bool tcp_receive(int socket, packet* package){

	byte_t buffer[MAX_BUFFER_SIZE];

	if(package == NULL){
		//fprintf("%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	size_t bytes_received = recv(socket, buffer, MAX_BUFFER_SIZE, 0);
	if(bytes_received == -1){
		//fprintf("%s: recv error\n", getTime());	
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
		//fprintf("%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	if(!serialize(package, buffer)){
		return FALSE;
	}

	size_t bytes_sent = send(socket, buffer, MAX_BUFFER_SIZE, 0);
	if(bytes_sent == -1){
		//fprintf("%s: send error\n", getTime());	
		return FALSE;
	}

	free_packet(package);
	free(buffer);

	return TRUE;
}


