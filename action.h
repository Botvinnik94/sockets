#ifndef __ACTION_H__
#define __ACTION_H__

#include "transfer.h"

void put_client_tcp(int socket, char *filename);
void put_server_tcp(int socket);

#endif
