#ifndef __TRANSFER_H__
#define __TRANSFER_H__

#include "packet.h"
#include <signal.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 516
#define TIMEOUT 5

bool tcp_receive(int socket, packet* package);
bool tcp_send(int socket, packet* package);

bool udp_receive(int socket, packet* package, struct sockaddr_in *clientaddr_in, int addrlen);
bool udp_send(int socket, packet* package, struct sockaddr_in *clientaddr_in, int addrlen);

void register_sigalrm();
void handler();

#endif
