#include "transfer.h"

extern FILE* logFile;

bool socket_receive(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type)
{
	byte_t buffer[MAX_BUFFER_SIZE];
	size_t bytes_received = 0;

	if(package == NULL){
		fprintf(logFile, "%s: package null at receive function\n", getTime());	

		return FALSE;
	}

	if( type == TCP )
	{
		bytes_received = recv(socket, buffer, MAX_BUFFER_SIZE, 0);
		if(bytes_received == -1 || bytes_received == 0){
            if( bytes_received == -1 )
                fprintf(logFile,"%s: recv error received\n", getTime());	
            else
                fprintf(logFile,"%s: recv shutdown received\n", getTime());	
            return FALSE;
	    }
	}
	else if( type == UDP)
	{
		alarm(TIMEOUT);
		/* When the alarm pops it will go back to the loop until max retries or unexpected error occurs */
		while ((bytes_received = recvfrom(socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)clientaddr_in, &addrlen)) == -1 && errno == EINTR){}
		alarm(0);
		if(bytes_received == -1 || bytes_received == 0){
		    if( bytes_received == -1 )
		        fprintf(logFile,"%s: recv error received\n", getTime());
		    else
		        fprintf(logFile,"%s: recv shutdown received\n", getTime());	
			return FALSE;
		}
	}
	else
	{
		fprintf(logFile, "%s: unexpected type(TCP(1)/UDP(2), got:%d)\n", getTime(), type);	
		return FALSE;
	}

	if(!unserialize(buffer, bytes_received, package)){
		return FALSE;	
	}

	return TRUE;
		
}

bool socket_send(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type)
{
	byte_t *buffer;
    size_t buffer_size = 0;
	size_t bytes_sent = 0;

	if(package == NULL){
		fprintf(logFile,"%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	if((buffer = serialize(package, &buffer_size)) == NULL)
		return FALSE;

	if( type == TCP )
		bytes_sent = send(socket, buffer, buffer_size, 0);
	else if( type == UDP )
		bytes_sent = sendto(socket, buffer, buffer_size, 0, (struct sockaddr*)clientaddr_in, addrlen);
	else
	{
		fprintf(logFile, "%s: unexpected type(TCP(1)/UDP(2), got:%d)\n", getTime(), type);	
		return FALSE;
	}

	if(bytes_sent == -1){
		fprintf(logFile,"%s: send error\n", getTime());	
		return FALSE;
	}

	/* Free the memory reserved in the serialize process */
	free_packet(package);
	free(buffer);

	return TRUE;

}


