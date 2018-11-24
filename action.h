#ifndef __ACTION_H__
#define __ACTION_H__

#include "transfer.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>


void put_client_tcp(int socket, char *filename);
void put_server_tcp(int socket, packet *package);
void shutdown_connection(int socket);

void get_client_tcp(int socket, char *filename);
void get_server_tcp(int socket);
#endif
