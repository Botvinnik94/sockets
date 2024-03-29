/*
 *      Práctica sockets 2018 - Redes I - TFTP
 *      action.c
 *
 *      Alfonso José Mateos Hoyos - 44059172G
 *      Gabino Luis Lazo - 71028058X
 */

#include "action.h"

extern FILE* logFile;

void put_client(int socket, char *filename, struct sockaddr_in *servaddr_in, int addrlen, int type)
{
	packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;
	char path[255] = "ficherosTFTPclient/";
    uint16_t nBloq = 1;

	/* Register SIGALRM if type is UDP */
	if( type == UDP )
		register_sigalrm();

	/* Open the file */
	strcat(path, filename);
	FILE *file = fopen(path, "r");
	if(file == NULL)
	{
		fprintf(logFile, "%s: file does not exist(filename=%s) at 'put client'\n", getTime(),path);
		return;
	}

	if(!build_RQ_packet(WRQ, filename, &package))
	{
		fprintf(logFile, "%s: error building request packet at 'put client'\n", getTime());
        fclose(file);
		return;
	}

	/* Send write request to the server */
	if(!socket_send(socket, &package, servaddr_in, addrlen, type)) 
	{
        fclose(file);
		return;
	}

	/* Wait for server ACK (protocol custom ACK)  */
    if( !socket_receive(socket, &package, servaddr_in, addrlen, type))
    {
        fclose(file);
		return;
    }

	/* Check the message received */
	if( package.opcode == ACK )
	{
		/* First nBloq must be 0 */
		if( package.ack_message.nBloq != 0 )
        {
            free_packet(&package);
            fprintf(logFile, "%s: Wrong initial ACK number in 'put client'\n", getTime());
            fclose(file);
            return;
        }
		
        free_packet(&package);

		/* Starts sending data to the server */
        while(TRUE)
		{
			n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file);
            if(!build_DATA_packet(buffer, n_read, nBloq, &package))
			{
				fprintf(logFile, "%s: error building data packet at 'put client'\n", getTime());
                fclose(file);
                return;		
            }

			/* Send the data read from the file */
            if(!socket_send(socket, &package, servaddr_in, addrlen, type)) {
                fclose(file);
                return;
            }
			/* Wait for server ACK */
            if( !socket_receive(socket, &package, servaddr_in, addrlen, type))
            {
                fclose(file);
                return;
            }

			/* An ACK was received */
            if( package.opcode == ACK )
            {	/* Check if the ACK is correct (nBloq must be the same) */
                if( nBloq != package.ack_message.nBloq )
                {
                    free_packet(&package);
                    fprintf(logFile, "%s: Wrong ACK number in 'put client'\n", getTime());
                    fclose(file);
                    return;
                }
            }
			/* An error was received */
            else if( package.opcode == ERR)
            {
                free_packet(&package);
                fprintf(logFile, "%s: Error received sending data in 'put client'\n", getTime());
                fclose(file);
                return;
            }
			/* Unexpected opcode was received */
            else
            {
                free_packet(&package);
                fprintf(logFile, "%s: Unexpected error in 'put client'\n", getTime());
                fclose(file);
                return;
            }

			/* If n_read is less than MAX_DATA_SIZE, it means the read is done
			if n_read = MAX_DATA_SIZE, it will iterate one more time to send an empty 
			message, so the receiver knows it finished reading */
            if(n_read < MAX_DATA_SIZE) break;

            free_packet(&package);
            nBloq++;
        }
	}
	else if( package.opcode == ERR )
    {
        fprintf(logFile, "%s: %s (put client)\n", getTime(), package.err_message.msg );
    }
    else
    {
        fprintf(logFile, "%s: Unexpected opcode at 'put client'\n",getTime());
    }

    free_packet(&package);
    fclose(file);

}

void put_server(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type)
{
	FILE *file;
	char path[255] = "ficherosTFTPserver/";
    uint16_t nBloq = 0;
    size_t data_size = 0;

	/* Register SIGALRM if type is UDP */
	if( type == UDP )
		register_sigalrm();

	/* Check if file already exists in server directory */
	strcat(path, package->request_message.filename);
    if( access(path, F_OK) == 0 )
    {
        free_packet(package);
        if( !build_ERR_packet(ERR_FILE_EXISTS, "File already exists at 'put_server'", package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }

        return;
    }
	/* If file doesn't exist, it continues */
	else
    {	
        free_packet(package);

        /* First ACK is built with nBloq = 0 */
        if( !build_ACK_packet(0, package) )
        {           
            fprintf(logFile, "%s: Error building ACK packet at 'put server'\n", getTime());
            return;
        }

		/* Send the first ACK */
        if( !socket_send(socket, package, clientaddr_in, addrlen, type) ) return;

        /* Opens the file to proceed with the writing */
        file = fopen(path, "w");
        if( file == NULL )
        {
            fprintf(logFile, "%s: Error creating file at 'put server'\n", getTime());
            return;
        }

        /* Iterates while not reveiving -1 (ERROR) or 0 (Shutdown) */
        while( socket_receive(socket, package, clientaddr_in, addrlen, type) )
        {	
            if (package->opcode == DATA)
            {
                data_size = package->data_message.data_size;
                /* Write the data received */
                fwrite(package->data_message.data, sizeof(byte_t), data_size, file);

                /* Error checking in every write. In case of error inform the client and exit */
                if( ferror(file) )
                {
                    fprintf(logFile, "%s: Error writing file at 'put server'\n", getTime());
                    fclose(file);

                    free_packet(package);
                    if(errno == ENOSPC || errno == EDQUOT || errno == ENOMEM)
                    {
                        if( !build_ERR_packet(ERR_FULL_DISK, "Not enough disk space in the server.", package) )
                        {           
                            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
                            return;
                        }
                    }
                    else
                    {
                        if( !build_ERR_packet(ERR_UNDEFINED, "Undefined error writing the file", package) )
                        {           
                            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
                            return;
                        }
                    }

                    if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
                    {
                        fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
                        return;
                    }

                    return;
                }

                nBloq = package->data_message.nBloq;
                free_packet(package);

                if( !build_ACK_packet(nBloq, package) )
                {           
                    fprintf(logFile, "%s: Error building ACK packet\n", getTime());
                    return;
                }

                if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
                {
                    fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
                    return;
                }

                /* if data size is less than MAX_DATA_SIZE, it means writing is done */
                if(data_size < MAX_DATA_SIZE) break;
                free_packet(package);
            }
            /* The server receives an unexpected operation. Inform the client and exit */
            else
            {
                fclose(file);

                fprintf(logFile, "%s: Unexpected opcode while writing data on 'put server'\n", getTime());
                if( !build_ERR_packet(ERR_ILLEGAL_OP, "Unexpected operation.", package) )
                {           
                    fprintf(logFile, "%s: Error building ERR packet\n", getTime());
                    return;
                }

                if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
                {
                    fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
                    return;
                }

                return;
            }
        }

        free_packet(package);
        fclose(file);
    }

}

void get_client(int socket, char *filename, struct sockaddr_in *servaddr_in, int addrlen, int type)
{

	packet package;
	byte_t buffer[MAX_DATA_SIZE];
	char path[255] = "ficherosTFTPclient/";
    uint16_t nBloq = 0;
    size_t data_size = 0;

	/* Register SIGALRM if type is UDP */
	if( type == UDP )
		register_sigalrm();

	/* Check if the desired file already exists */
	strcat(path, filename);
    if( access(path, F_OK) == 0 )
    {   
        fprintf(logFile, "%s: File already exists at 'get client'(filename=%s)\n", getTime(), path);
		return; 
    }

	if(!build_RQ_packet(RRQ, filename, &package))
	{
		fprintf(logFile, "%s: Error request packet at 'get client'\n", getTime());
		return;
	}

    /* Send write request to the server */
	if(!socket_send(socket, &package, servaddr_in, addrlen, type)) return;

    /* Wait for server ACK (our custom ACK)  */
    if( !socket_receive(socket, &package, servaddr_in, addrlen, type)) return;

    if( package.opcode == ACK )
    {
        /* Open the file */
        FILE *file = fopen(path, "w");
        if(file == NULL)
		{
            free_packet(&package);
            fprintf(logFile, "%s: error opening file (filename=%s) at 'get client'\n", getTime(),path);
            return;
        }
        free_packet(&package);

        while( socket_receive(socket, &package, servaddr_in, addrlen, type) )
        {
            if(package.opcode == DATA)
            {
                data_size = package.data_message.data_size;

                if( fwrite(package.data_message.data, sizeof(byte_t), data_size, file) < data_size )
                {
                    fprintf(logFile, "%s: Error writing file at 'get client'\n", getTime());
                    break;
                }

                nBloq = package.data_message.nBloq;
                free_packet(&package);

                if( !build_ACK_packet(nBloq, &package) )
                {           
                    fprintf(logFile, "%s: Error building ACK packet\n", getTime());
                    return;
                }

                if( !socket_send(socket, &package, servaddr_in, addrlen, type) )
                {
                    fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
                    return;
                }

                if(data_size < MAX_DATA_SIZE) break;
                free_packet(&package);
            }
            else
            {
                fprintf(logFile, "%s: Unexpected opcode while writing data on 'put server'\n", getTime());
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

    free_packet(&package);
}

void get_server(int socket, packet *package, struct sockaddr_in *clientaddr_in, int addrlen, int type )
{
	FILE *file;
    size_t n_read;
	char path[255] = "ficherosTFTPserver/";
    byte_t buffer[MAX_DATA_SIZE];
    uint16_t nBloq = 1;

	/* Register SIGALRM if type is UDP */
	if( type == UDP )
		register_sigalrm();

    strcat(path, package->request_message.filename);
    /* File has to exist so the server can send the file */
    if( access(path, F_OK) != 0 )
    {
        if( !build_ERR_packet(ERR_FILE_NOT_FOUND, "File does not exist", package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }

        return;
    }
    else
    {	
        free_packet(package);

        /* The first ACK must be nBloq=0 */
        if( !build_ACK_packet(0, package) )
        {           
            fprintf(logFile, "%s: Error building ACK packet\n", getTime());
            return;
        }

        if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
        {
            fprintf(logFile, "%s: Error sending ACK packet\n", getTime());
            return;
        }

        /* Opens the file to proceed with the reading */
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
			
			/* Send the data read from the file */
            if(!socket_send(socket, package, clientaddr_in, addrlen, type))
            {
                fclose(file);
                return;
            }

			/* Wait for the ACK from the client */
            if( !socket_receive(socket, package, clientaddr_in, addrlen, type))
            {
                fclose(file);
                return;
            }

			/* If it was an ACK, checks the ACK number(nBloq) */
            if( package->opcode == ACK )
            {
                if( nBloq != package->ack_message.nBloq )
                {
                    free_packet(package);
                    fprintf(logFile, "%s: Wrong ACK number in 'put client udp'\n", getTime());
                    fclose(file);

                    if( !build_ERR_packet(ERR_ILLEGAL_OP, "ACK block number does not match", package) )
                    {           
                        fprintf(logFile, "%s: Error building ERR packet\n", getTime());
                        return;
                    }

                    if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
                    {
                        fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
                        return;
                    }

                    return;
                }
            }
			/* An error message was received */
            else if( package->opcode == ERR)
            {
                free_packet(package);
                fprintf(logFile, "%s: Error received sending data in 'put client udp'\n", getTime());
                fclose(file);
                return;
            }
			/* Unexpected opcode received */
            else
            {
                free_packet(package);
                fprintf(logFile, "%s: Unexpected error in 'put client udp'\n", getTime());
                fclose(file);

                if( !build_ERR_packet(ERR_ILLEGAL_OP, "Unexpected operation", package) )
                {           
                    fprintf(logFile, "%s: Error building ERR packet\n", getTime());
                    return;
                }

                if( !socket_send(socket, package, clientaddr_in, addrlen, type) )
                {
                    fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
                    return;
                }

                return;
            }

            if(n_read < MAX_DATA_SIZE) break;

            free_packet(package);
            nBloq++;
        }

        fclose(file);

	}

    free_packet(package);
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
    /* Register SIGALRM in order to make timeouts for UDP receives */
    struct sigaction vec;
    vec.sa_handler = (void *) timeout_handler;
    vec.sa_flags = 0;

    if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1)
    {
        fprintf(logFile,"%s: unable to register the SIGALRM signal\n", getTime());
		fclose(logFile);
        exit(1);
    }
}

void timeout_handler()
{
	static int retries_left = NUM_RETRIES;

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
