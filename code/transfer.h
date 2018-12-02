/*
 *      Práctica sockets 2018 - Redes I - TFTP
 *      transfer.h
 *
 *      Alfonso José Mateos Hoyos - 44059172G
 *      Gabino Luis Lazo - 71028058X
 */

#ifndef __TRANSFER_H__
#define __TRANSFER_H__

#include "packet.h"
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 516
#define TIMEOUT 20

#define UDP 1
#define TCP 2

bool socket_receive(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type);
bool socket_send(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type);

#endif
