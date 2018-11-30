#ifndef __ACTION_H__
#define __ACTION_H__

#include "transfer.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* Write functions */
void put_client(int socket, char *filename, struct sockaddr_in *servaddr_in, int addrlen, int type);
void put_server(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type);

/* Read functions */
void get_client(int socket, char *filename, struct sockaddr_in *servaddr_in, int addrlen, int type);
void get_server(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type );

/* Aux functions */

void shutdown_connection(int socket);
void register_sigalrm();
void handler();


#endif
