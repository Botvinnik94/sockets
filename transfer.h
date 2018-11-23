#ifndef __TRANSFER_H__
#define __TRANSFER_H__

#include "packet.h"

#define MAX_BUFFER_SIZE 1024

bool tcp_receive(int socket, packet* package);
bool tcp_send(int socket, packet* package);

#endif
