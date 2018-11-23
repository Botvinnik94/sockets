#include "action.h"


void put_client_tcp(int socket, char *filename){

	packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;


	FILE *file = fopen(filename, "r");
	if(file == NULL){
		//fprintf(logFile, "%s: file does not exist\n", getTime());
		shutdown_connection(socket);
		return;
	}

	if(!build_RQ_packet(WRQ, filename, &package)){
		shutdown_connection(socket);
		return;
	}

	if(!tcp_send(socket, &package)) {
		shutdown_connection(socket);
		return;
	}

	while(n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file)){
		if(!build_DATA_packet(buffer, n_read, 0, &package)) {
			shutdown_connection(socket);
			return;		
		}

		if(!tcp_send(socket, &package)) {
			shutdown_connection(socket);
			return;
		}
	}

	shutdown_connection(socket);

}

void put_server_tcp(int socket){
	
	
}

void shutdown_connection(int socket){
	if (shutdown(socket, SHUT_WR) == -1) {
		//fprintf(logFile, "%s: %s: unable to shutdown socket\n", getTime(), argv[0]);
		exit(1);
	}
}
