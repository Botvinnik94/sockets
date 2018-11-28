#include "action.h"

extern FILE* logFile;

void put_client_tcp(int socket, char *filename){

	packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;
	char path[255] = "ficherosTFTPclient/";
    long nBloq = 1;

    /* Opens the file */
	strcat(path, filename);
	FILE *file = fopen(path, "r");
	if(file == NULL){
		fprintf(logFile, "%s: file does not exist(filename=%s)\n", getTime(),path);
		shutdown_connection(socket);
		return;
	}

	if(!build_RQ_packet(WRQ, filename, &package)){
		shutdown_connection(socket);
        fclose(file);
		return;
	}

    /* Sends write request to the server */
	if(!tcp_send(socket, &package)) {
		shutdown_connection(socket);
        fclose(file);
		return;
	}

    /* Waits for server ACK (our custom ACK)  */
    if( !tcp_receive(socket, &package))
    {
        shutdown_connection(socket);
        fclose(file);
		return;
    }

    if( package.opcode == ACK )
    {
        if( package.ack_message.nBloq != 0 )
        {
            fprintf(logFile, "%s: Wrong initial ACK number in 'put client TCP'\n", getTime());
            fclose(file);
             return;
        }

        /* Starts sending data to the server */
        while(TRUE){

            n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file);
            if(!build_DATA_packet(buffer, n_read, nBloq, &package)) {
                shutdown_connection(socket);
                fclose(file);
                return;		
            }

            if(!tcp_send(socket, &package)) {
                shutdown_connection(socket);
                fclose(file);
                return;
            }

            if( !tcp_receive(socket, &package))
            {
                shutdown_connection(socket);
                fclose(file);
                return;
            }

            if( package.opcode == ACK )
            {
                if( nBloq != package.ack_message.nBloq )
                {
                    shutdown_connection(socket);
                    fprintf(logFile, "%s: Wrong ACK number in 'put client tcp'\n", getTime());
                    fclose(file);
                    return;
                }
            }
            else if( package.opcode == ERR)
            {
                shutdown_connection(socket);
                fprintf(logFile, "%s: Error received sending data in 'put client tcp'\n", getTime());
                fclose(file);
                return;
            }
            else
            {
                shutdown_connection(socket);
                fprintf(logFile, "%s: Unexpected error in 'put client tcp'\n", getTime());
                fclose(file);
                return;
            }

            if(n_read < MAX_DATA_SIZE) break;
            nBloq++;
        }
    }
    else if( package.opcode == ERR )
    {
        fprintf(logFile, "%s: %s\n", getTime(), package.err_message.msg );
    }
    else
    {
        fprintf(logFile, "%s: Unexpected opcode at 'put client'\n",getTime());
    }

	shutdown_connection(socket);
    fclose(file);

}

void put_server_tcp(int socket, packet *package)
{
    FILE *file;
	char path[255] = "ficherosTFTPserver/";

    strcat(path, package->request_message.filename);
    if( access(path, F_OK) == 0 )
    {
        if( !build_ERR_packet(ERR_FILE_EXISTS, "File already exists", package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !tcp_send(socket, package) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }

        return;
    }

    else
    {
        if( !build_ACK_packet(0, package) )
        {           
            fprintf(logFile, "%s: Error building ACK packet\n", getTime());
            return;
        }

        if( !tcp_send(socket, package) )
        {
            fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
            return;
        }

        /* Opens the file to proceed with the writing */
        file = fopen(path, "w");
        if( file == NULL )
        {
            fprintf(logFile, "%s: Error creating file at 'put server'\n", getTime());
            return;
        }

        /* Iterates while not reveiving -1 (ERROR) or 0 (Shutdown) */
        while( tcp_receive(socket, package) )
        {
            if( fwrite(package->data_message.data, sizeof(byte_t), package->data_message.data_size, file) < package->data_message.data_size )
            {
                fprintf(logFile, "%s: Error writing file at 'put server'\n", getTime());
                break;
            }

            if( !build_ACK_packet(package->data_message.nBloq, package) )
            {
                fprintf(logFile, "%s: Error building ACK packet\n", getTime());
                return;
            }

            if( !tcp_send(socket, package) )
            {
                fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
                return;
            }

            if(package->data_message.data_size < MAX_DATA_SIZE) break;
        }

        fclose(file);
        //shutdown_connection(socket); not necessary? ASK 
    }

}

void get_client_tcp(int socket, char *filename) {

    packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;
	char path[255] = "ficherosTFTPclient/";

    strcat(path, filename);
    if( access(path, F_OK) == 0 )
    {    
        fprintf(logFile, "%s: File already exists at 'get client tcp'(filename=%s)\n", getTime(), path);
        shutdown_connection(socket);
        return;
    }

    if(!build_RQ_packet(RRQ, filename, &package)){
		shutdown_connection(socket);
		return;
	}

    /* Sends write request to the server */
	if(!tcp_send(socket, &package)) {
		shutdown_connection(socket);
		return;
	}

    /* Waits for server ACK (our custom ACK)  */
    if( !tcp_receive(socket, &package))
    {
        shutdown_connection(socket);
		return;
    }

    if( package.opcode == ACK )
    {
        /* Opens the file */
        FILE *file = fopen(path, "w");
        if(file == NULL){
            fprintf(logFile, "%s: error opening file (filename=%s)\n", getTime(),path);
            shutdown_connection(socket);
            return;
        }

        while( tcp_receive(socket, &package) )
        {
            if( fwrite(package.data_message.data, sizeof(byte_t), package.data_message.data_size, file) < package.data_message.data_size )
            {
                fprintf(logFile, "%s: Error writing file at 'get client tcp'\n", getTime());
                break;
            }
        }

        fclose(file);
    }
    else if( package.opcode == ERR )
    {
        fprintf(logFile, "%s: %s\n", getTime(), package.err_message.msg );
    }
    else
    {
        fprintf(logFile, "%s: Unexpected opcode at 'get client tcp'\n",getTime());
    }

    shutdown_connection(socket);
}

void get_server_tcp(int socket, packet *package)
{
    FILE *file;
    size_t n_read;
	char path[255] = "ficherosTFTPserver/";
    byte_t buffer[MAX_DATA_SIZE];

    strcat(path, package->request_message.filename);
    /* File have to exist so the server can send the file */
    if( access(path, F_OK) != 0 )
    {
        if( !build_ERR_packet(ERR_FILE_EXISTS, "File already exists", package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !tcp_send(socket, package) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }

        return;
    }

    else
    {
        if( !build_ACK_packet(0, package) )
        {           
            fprintf(logFile, "%s: Error building ACK packet\n", getTime());
            return;
        }

        if( !tcp_send(socket, package) )
        {
            fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
            return;
        }

        /* Opens the file to proceed with the writing */
        file = fopen(path, "r");
        if( file == NULL )
        {
            fprintf(logFile, "%s: Error opening file at 'get server'\n", getTime());
            return;
        }

        /* Starts sending data to the server */
        while(n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file)){
            if(!build_DATA_packet(buffer, n_read, 0, package)) {
                shutdown_connection(socket);
                fclose(file);
                return;		
            }

            if(!tcp_send(socket, package)) {
                shutdown_connection(socket);
                fclose(file);
                return;
            }
        }

        fclose(file);
        //shutdown_connection(socket); not necessary? ASK 
    }
}

void put_client_udp(int socket, char *filename, struct sockaddr_in *servaddr_in, int addrlen) 
{
    packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;
	char path[255] = "ficherosTFTPclient/";
    long nBloq = 1;

    register_sigalrm();

    /* Opens the file */
	strcat(path, filename);
	FILE *file = fopen(path, "r");
	if(file == NULL){
		fprintf(logFile, "%s: file does not exist(filename=%s)\n", getTime(),path);
		return;
	}

	if(!build_RQ_packet(WRQ, filename, &package)){
        fclose(file);
		return;
	}

    /* Sends write request to the server */
	if(!udp_send(socket, &package, servaddr_in, addrlen)) {
        fclose(file);
		return;
	}

    /* Waits for server ACK (our custom ACK)  */
    if( !udp_receive(socket, &package, servaddr_in, addrlen))
    {
        fclose(file);
		return;
    }

    if( package.opcode == ACK )
    {
        if( package.ack_message.nBloq != 0 )
        {
            fprintf(logFile, "%s: Wrong initial ACK number in 'put client udp'\n", getTime());
            fclose(file);
             return;
        }

        /* Starts sending data to the server */
        while(TRUE){

            n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file);
            if(!build_DATA_packet(buffer, n_read, nBloq, &package))
            {
                fclose(file);
                return;		
            }

            if(!udp_send(socket, &package, servaddr_in, addrlen))
            {
                fclose(file);
                return;
            }

            if( !udp_receive(socket, &package, servaddr_in, addrlen))
            {
                fclose(file);
                return;
            }

            if( package.opcode == ACK )
            {
                if( nBloq != package.ack_message.nBloq )
                {
                    fprintf(logFile, "%s: Wrong ACK number in 'put client udp'\n", getTime());
                    fclose(file);
                    return;
                }
            }
            else if( package.opcode == ERR)
            {
                fprintf(logFile, "%s: Error received sending data in 'put client udp'\n", getTime());
                fclose(file);
                return;
            }
            else
            {
                fprintf(logFile, "%s: Unexpected error in 'put client udp'\n", getTime());
                fclose(file);
                return;
            }

            if(n_read < MAX_DATA_SIZE) break;
            nBloq++;
        }
    }
    else if( package.opcode == ERR )
    {
        fprintf(logFile, "%s: %s\n", getTime(), package.err_message.msg );
    }
    else
    {
        fprintf(logFile, "%s: Unexpected opcode at 'put client'\n",getTime());
    }

    fclose(file);
}


void put_server_udp(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen)
{
    FILE *file;
	char path[255] = "ficherosTFTPserver/";

    register_sigalrm();

    strcat(path, package->request_message.filename);
    if( access(path, F_OK) == 0 )
    {
        if( !build_ERR_packet(ERR_FILE_EXISTS, "File already exists", package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !udp_send(socket, package, clientaddr_in, addrlen) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }

        return;
    }

    else
    {
        if( !build_ACK_packet(0, package) )
        {           
            fprintf(logFile, "%s: Error building ACK packet\n", getTime());
            return;
        }

        if( !udp_send(socket, package, clientaddr_in, addrlen) )
        {
            fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
            return;
        }

        /* Opens the file to proceed with the writing */
        file = fopen(path, "w");
        if( file == NULL )
        {
            fprintf(logFile, "%s: Error creating file at 'put server'\n", getTime());
            return;
        }

        /* Iterates while not receiving*/
        while( udp_receive(socket, package, clientaddr_in, addrlen) )
        {
            if( fwrite(package->data_message.data, sizeof(byte_t), package->data_message.data_size, file) < package->data_message.data_size )
            {
                fprintf(logFile, "%s: Error writing file at 'put server'\n", getTime());
                fclose(file);
                break;
            }

            if( !build_ACK_packet(package->data_message.nBloq, package) )
            {           
                fprintf(logFile, "%s: Error building ACK packet\n", getTime());
                return;
            }

            if( !udp_send(socket, package, clientaddr_in, addrlen) )
            {
                fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
                return;
            }

            if(package->data_message.data_size < MAX_DATA_SIZE) break;
        }

        fclose(file);
    }
}

void get_client_udp(int socket, char *filename, struct sockaddr_in *servaddr_in, int addrlen) {

    packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;
	char path[255] = "ficherosTFTPclient/";

    strcat(path, filename);
    if( access(path, F_OK) == 0 )
    {    
        fprintf(logFile, "%s: File already exists at 'get client tcp'(filename=%s)\n", getTime(), path);
        return;
    }

    if(!build_RQ_packet(RRQ, filename, &package)){
		return;
	}

    /* Sends write request to the server */
	if(!udp_send(socket, &package, servaddr_in, addrlen)) {
		return;
	}

    /* Waits for server ACK (our custom ACK)  */
    if( !udp_receive(socket, &package, servaddr_in, addrlen))
    {
		return;
    }

    if( package.opcode == ACK )
    {
        /* Opens the file */
        FILE *file = fopen(path, "w");
        if(file == NULL){
            fprintf(logFile, "%s: error opening file (filename=%s)\n", getTime(),path);
            return;
        }

        while( udp_receive(socket, &package, servaddr_in, addrlen) )
        {
            if( fwrite(package.data_message.data, sizeof(byte_t), package.data_message.data_size, file) < package.data_message.data_size )
            {
                fprintf(logFile, "%s: Error writing file at 'get client tcp'\n", getTime());
                break;
            }

            if( !build_ACK_packet(package.data_message.nBloq, &package) )
            {           
                fprintf(logFile, "%s: Error building ACK packet\n", getTime());
                return;
            }

            if( !udp_send(socket, &package, servaddr_in, addrlen) )
            {
                fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
                return;
            }

            if(package.data_message.data_size < MAX_DATA_SIZE) break;
        }

        fclose(file);
    }
    else if( package.opcode == ERR )
    {
        fprintf(logFile, "%s: %s\n", getTime(), package.err_message.msg );
    }
    else
    {
        fprintf(logFile, "%s: Unexpected opcode at 'get client tcp'\n",getTime());
    }
}

void get_server_udp(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen)
{
    FILE *file;
    size_t n_read;
	char path[255] = "ficherosTFTPserver/";
    byte_t buffer[MAX_DATA_SIZE];
    long nBloq = 1;

    strcat(path, package->request_message.filename);
    /* File have to exist so the server can send the file */
    if( access(path, F_OK) != 0 )
    {
        if( !build_ERR_packet(ERR_FILE_EXISTS, "File already exists", package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !udp_send(socket, package, clientaddr_in, addrlen) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }

        return;
    }

    else
    {
        if( !build_ACK_packet(0, package) )
        {           
            fprintf(logFile, "%s: Error building ACK packet\n", getTime());
            return;
        }

        if( !udp_send(socket, package, clientaddr_in, addrlen) )
        {
            fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
            return;
        }

        /* Opens the file to proceed with the writing */
        file = fopen(path, "r");
        if( file == NULL )
        {
            fprintf(logFile, "%s: Error opening file at 'get server'\n", getTime());
            return;
        }

        /* Starts sending data to the server */
        while(TRUE){

            n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file);
            if(!build_DATA_packet(buffer, n_read, nBloq, package))
            {
                fclose(file);
                return;		
            }

            if(!udp_send(socket, package, clientaddr_in, addrlen))
            {
                fclose(file);
                return;
            }

            if( !udp_receive(socket, package, clientaddr_in, addrlen))
            {
                fclose(file);
                return;
            }

            if( package->opcode == ACK )
            {
                if( nBloq != package->ack_message.nBloq )
                {
                    fprintf(logFile, "%s: Wrong ACK number in 'put client udp'\n", getTime());
                    fclose(file);
                    return;
                }
            }
            else if( package->opcode == ERR)
            {
                fprintf(logFile, "%s: Error received sending data in 'put client udp'\n", getTime());
                fclose(file);
                return;
            }
            else
            {
                fprintf(logFile, "%s: Unexpected error in 'put client udp'\n", getTime());
                fclose(file);
                return;
            }

            if(n_read < MAX_DATA_SIZE) break;
            nBloq++;
        }

        fclose(file);
        //shutdown_connection(socket); not necessary? ASK 
    }
}

void shutdown_connection(int socket)
{
	if (shutdown(socket, SHUT_WR) == -1) 
    {
		fprintf(logFile, "%s: unable to shutdown socket\n", getTime());
		exit(1);
	}
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