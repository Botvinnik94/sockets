#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include "action.h"

#define PORT 17278
#define TAM_BUFFER 10

FILE * logFile;
void clientcp( int socket, char *argv[] );
void clientudp( int socket, char *argv[] );

int main( int argc, char *argv[] )
{
	int s; 							/* Socket descriptor */	

	/* Open the client log file */
	if ( (logFile = fopen("cliente.txt", "a")) == NULL )
	{
		fprintf(logFile, "%s: Unable to create log file. Exiting...\n", getTime());
		exit(1);	
	}
	
	/* Check arguments */
	if ( argc != 5 || (strcmp(argv[2],"UDP") && strcmp(argv[2],"TCP")) || (strcmp(argv[3],"l") && strcmp(argv[3],"e")) )
	{
		fprintf(logFile, "%s: Usage:  <./%s> <nameserver> <TCP/UDP> <[r|l]> <filename.txt>\n", getTime(), argv[0]);
		fclose(logFile);
		exit(1);
	}

	/* Create the socket */
	if ( (s = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
	{
		fprintf(logFile, "%s: %s: unable to create socket\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	}

	/* Check if it's a TCP or UDP client */
	if( !strcmp(argv[2], "TCP") )
		clientcp( s, argv );
	else
		clientudp( s, argv ); /* TO DO */

	
	/* Print message indicating completion of task. */
	fprintf(logFile, "All done at %s", getTime());
	fclose(logFile);
	
	return 0;
}

void clientcp( int socket, char *argv[] )
{
	struct addrinfo hints, *res;
	struct sockaddr_in myaddr_in;	/* For local socket address */
	struct sockaddr_in servaddr_in;	/* For server socket address */
	int addrlen, i, j, errcode;
	
	/* Clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	/* Set up the peer address to which we will connect. */
	servaddr_in.sin_family = AF_INET;

	/* Get the host information for the hostname that the user passed in. */
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;
	
	if ( (errcode = getaddrinfo (argv[1], NULL, &hints, &res)) != 0 )
	{
		/* Name was not found. Return a special value signifying the error. */
		fprintf(logFile, "%s: %s: Couldn't resolve %s IP\n", getTime(), argv[0], argv[1]);
        fclose(logFile);
		exit(1);
	}
	else
	{
		/* Copy address of host */
		servaddr_in.sin_addr = ( (struct sockaddr_in *) res->ai_addr)->sin_addr;
	}

	freeaddrinfo(res);
	
	/* Server port */
	servaddr_in.sin_port = htons(PORT);

	/* Try to connect to the remote server at the address which was just built into peeraddr. */
	if ( connect(socket, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1 )
	{
		fprintf(logFile, "%s: %s: unable to connect to remote\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	}

	/* Since the connect call assigns a free address to the local end of this connection,
		let's use getsockname to see what it assigned.  Note that addrlen needs to be passed 
		in as a pointer, because getsockname returns the actual length of the address. */
	addrlen = sizeof(struct sockaddr_in);
	if ( getsockname(socket, (struct sockaddr *)&myaddr_in, &addrlen) == -1 )
	{
		fprintf(logFile, "%s: %s: unable to read socket address\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	 }

	fprintf(logFile, "%s: Connected to %s on port %u\n", getTime(), argv[1], ntohs(myaddr_in.sin_port));

	if(!strcmp(argv[3], "r")){
		get_client_tcp(socket, argv[4]);
	}
	else{
		put_client_tcp(socket, argv[4]);
	}

}

void clientudp( int socket, char *argv[] )
{
	struct sockaddr_in myaddr_in;	/* For local socket address */
	struct sockaddr_in servaddr_in;	/* For server socket address */
    struct addrinfo hints, *res;
    int errcode, addrlen;

	/* Clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

    myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port = 0;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(socket, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(logFile, "%s: %s unable to bind socket\n", getTime(), argv[0]);
		fclose(logFile);
		exit(1);
	   }
    addrlen = sizeof(struct sockaddr_in);
    if (getsockname(socket, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
            perror(argv[0]);
            fprintf(logFile, "%s: %s unable to read socket address\n", getTime(), argv[0]);
			fclose(logFile);
            exit(1);
    }


            /* The port number must be converted first to host byte
             * order before printing.  On most hosts, this is not
             * necessary, but the ntohs() call is included here so
             * that this program could easily be ported to a host
             * that does require it.
             */
    fprintf(logFile, "%s: Connected to %s on port %u\n", getTime(), argv[1], ntohs(myaddr_in.sin_port));

	/* Set up the server address. */
	servaddr_in.sin_family = AF_INET;
		/* Get the host information for the server's hostname that the
		 * user passed in.
		 */
      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_INET;
 	 /* esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
    errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
    if (errcode != 0){
			/* Name was not found.  Return a
			 * special value signifying the error. */
		fprintf(logFile, "%s: %s No es posible resolver la IP de %s\n", getTime(),
				argv[0], argv[1]);
		fclose(logFile);
		exit(1);
      }
    else {
			/* Copy address of host */
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	 }
     freeaddrinfo(res);
     /* puerto del servidor en orden de red*/
	 servaddr_in.sin_port = htons(PORT);

   
	
	if(!strcmp(argv[3], "e")){
		put_client_udp(socket, argv[4], &myaddr_in, addrlen);
	}
	else{
		//TODO
	}
}
