#include "transfer.h"

extern FILE* logFile;

bool tcp_receive(int socket, packet* package){

	byte_t buffer[MAX_BUFFER_SIZE];

	if(package == NULL){
		fprintf(logFile, "%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	size_t bytes_received = recv(socket, buffer, MAX_BUFFER_SIZE, 0);
	if(bytes_received == -1 || bytes_received == 0){
        if( bytes_received == -1 )
		    fprintf(logFile,"%s: recv error received\n", getTime());	
        else
            fprintf(logFile,"%s: recv shutdown received\n", getTime());	
		return FALSE;
	}

	if(!unserialize(buffer, bytes_received, package)){
		return FALSE;	
	}

	return TRUE;
}

bool tcp_send(int socket, packet* package){
	
	byte_t *buffer;
    size_t buffer_size = 0;

	if(package == NULL){
		fprintf(logFile,"%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	if((buffer = serialize(package, &buffer_size)) == NULL) {
		return FALSE;
	}

	size_t bytes_sent = send(socket, buffer, buffer_size, 0);

	if(bytes_sent == -1){
		fprintf(logFile,"%s: send error\n", getTime());	
		return FALSE;
	}

	free_packet(package);
	free(buffer);

	return TRUE;
}

bool udp_receive(int socket, packet* package, struct sockaddr_in *clientaddr_in, int addrlen)
{
    register_sigalrm();

    byte_t buffer[MAX_BUFFER_SIZE];

	if(package == NULL){
		fprintf(logFile, "%s: package null at receive function\n", getTime());	
		return FALSE;
	}

    alarm(TIMEOUT);
	size_t bytes_received = recvfrom(socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)clientaddr_in, &addrlen);
    alarm(0);
	if(bytes_received == -1 || bytes_received == 0){
        if( bytes_received == -1 )
		    fprintf(logFile,"%s: recv error received\n", getTime());	
        else
            fprintf(logFile,"%s: recv shutdown received\n", getTime());	
		return FALSE;
	}

	if(!unserialize(buffer, bytes_received, package)){
		return FALSE;	
	}

	return TRUE;

}

bool udp_send(int socket, packet* package, struct sockaddr_in *clientaddr_in, int addrlen)
{
    byte_t *buffer;
    size_t buffer_size = 0;

	if(package == NULL){
		fprintf(logFile,"%s: package null at receive function\n", getTime());	
		return FALSE;
	}

	if((buffer = serialize(package, &buffer_size)) == NULL) {
		return FALSE;
	}

	size_t bytes_sent = sendto(socket, buffer, buffer_size, 0, (struct sockaddr*)clientaddr_in, addrlen);

	if(bytes_sent == -1){
		fprintf(logFile,"%s: send error\n", getTime());	
		return FALSE;
	}

	free_packet(package);
	free(buffer);

	return TRUE;
}

void register_sigalrm()
{
    /* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
    struct sigaction vec;
    vec.sa_handler = (void *) handler;
    vec.sa_flags = 0;

    if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1)
    {
        fprintf(logFile,"%s: unable to register the SIGALRM signal\n", getTime());
		fclose(logFile);
        exit(1);
    }
}

void handler()
{
	static int retries_left = 5;

    if( --retries_left )
    {
        fprintf(logFile, "%s: Retries left = %d\n", getTime(), retries_left);
        alarm(TIMEOUT);
    }
    else
    {
        fprintf(logFile, "%s: Ran out retries, exiting...\n", getTime());
        exit(1);
    }

}

