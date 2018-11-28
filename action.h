#ifndef __ACTION_H__
#define __ACTION_H__

#include "transfer.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* TCP functions */
void put_client_tcp(int socket, char *filename);
void put_server_tcp(int socket, packet *package);

void get_client_tcp(int socket, char *filename);
void get_server_tcp(int socket, packet *package);

/* UDP functions */
void put_client_udp(int socket, char *filename, struct sockaddr_in *clientaddr_in, int addrlen);
void put_server_udp(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen);

void get_client_udp(int socket, char *filename, struct sockaddr_in *clientaddr_in, int addrlen);
void get_server_udp(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen);

void shutdown_connection(int socket);
void register_sigalrm();
void handler();


#endif
