#include "action.h"

extern FILE* logFile;

void put_client_tcp(int socket, char *filename){

	packet package;
	byte_t buffer[MAX_DATA_SIZE];
	size_t n_read;
	char path[] = "ficherosTFTPclient/";

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
        /* Starts sending data to the server */
        while(n_read = fread(buffer, sizeof(byte_t), MAX_DATA_SIZE, file)){
            if(!build_DATA_packet(buffer, n_read, 0, &package)) {
                shutdown_connection(socket);
                fclose(file);
                return;		
            }

            if(!tcp_send(socket, &package)) {
                shutdown_connection(socket);
                fclose(file);
                return;
            }
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
	char path[] = "ficherosTFTPserver/";

    if( access(package->request_message.filename, F_OK) == 0 )
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
        strcat(path, package->request_message.filename);

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

        /* Waits for DATA packages *//*
        if( !tcp_receive(socket, package) )
        {
            fprintf(logFile, "%s: Error receiving DATA packet\n", getTime());
            return;
        }*/

        /* Iterates while not reveiving -1 (ERROR) or 0 (Shutdown) */
        while( tcp_receive(socket, package) )
        {
            //write(package->data_message.data, sizeof(byte_t), package->data_message.data_size, file);
            if( fwrite(package->data_message.data, sizeof(byte_t), package->data_message.data_size, file) < package->data_message.data_size )
            {
                fprintf(logFile, "%s: Error writing file at 'put server'\n", getTime());
                break;
            }

                fprintf(logFile, "%s: ITERACION REALIZADA'\n", getTime());

        }

        fclose(file);
        //shutdown_connection(socket);
    }

}

void shutdown_connection(int socket){
	if (shutdown(socket, SHUT_WR) == -1) {
		fprintf(logFile, "%s: unable to shutdown socket\n", getTime());
		exit(1);
	}
}
