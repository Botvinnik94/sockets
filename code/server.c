/*
 *      Práctica sockets 2018 - Redes I - TFTP
 *      server.c
 *
 *      Alfonso José Mateos Hoyos - 44059172G
 *      Gabino Luis Lazo - 71028058X
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "action.h"

#define PUERTO          9172
#define ADDRNOTFOUND0   0xffffffff
#define BUFFERSIZE      1024
#define TAM_BUFFER      10
#define MAXHOST         128

extern int errno;

void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s, char * buffer, size_t buffer_size, struct sockaddr_in clientaddr_in);
void errout(char *);		/* declare error out routine */

FILE * logFile;
int FIN = 0;
void finalizar(){ FIN = 1; }

int main(argc, argv)
int argc;
char *argv[];
{

    int s_TCP, s_UDP;		/* connected socket descriptor */
    int ls_TCP;				/* listen socket descriptor */
    
    int cc;				    /* contains the number of bytes read */
     
    struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */
    
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in clientaddr_in;	/* for peer socket address */
	int addrlen;
	
    fd_set readmask;
    int numfds,s_mayor;
    
    char buffer[BUFFERSIZE];	/* buffer for packets to be read into */
    
    struct sigaction vec;

    /* Open the server log file */
    logFile = fopen("server.txt", "a");
	if(logFile == NULL){
		fprintf(logFile, "%s: Unable to create log file. Exiting...\n", getTime());
		exit(1);
	}

	/* Create the listen socket. */
	ls_TCP = socket (AF_INET, SOCK_STREAM, 0);
	if (ls_TCP == -1) {
		perror(argv[0]);
		fprintf(logFile, "%s: %s: unable to create socket TCP\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	}
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
   	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    addrlen = sizeof(struct sockaddr_in);

		/* Set up address structure for the listen socket. */
	myaddr_in.sin_family = AF_INET;
		/* The server should listen on the wildcard address,
		 * rather than its own internet address.  This is
		 * generally good practice for servers, because on
		 * systems which are connected to more than one
		 * network at once will be able to have one server
		 * listening on all networks at once.  Even when the
		 * host is connected to only one network, this is good
		 * practice, because it makes the server program more
		 * portable.
		 */
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	myaddr_in.sin_port = htons(PUERTO);

	/* Bind the listen address to the socket. */
	if (bind(ls_TCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(logFile, "%s: %s: unable to bind address TCP\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	}
		/* Initiate the listen on the socket so remote users
		 * can connect.  The listen backlog is set to 5, which
		 * is the largest currently supported.
		 */
	if (listen(ls_TCP, 5) == -1) {
		perror(argv[0]);
		fprintf(logFile, "%s: %s: unable to listen on socket\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	}
	
	
	/* Create the socket UDP. */
	s_UDP = socket (AF_INET, SOCK_DGRAM, 0);
	if (s_UDP == -1) {
		perror(argv[0]);
		fprintf(logFile, "%s: %s: unable to create socket UDP\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	   }
	/* Bind the server's address to the socket. */
	if (bind(s_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) 
	{
		perror(argv[0]);
		fprintf(logFile, "%s: %s: unable to bind address UDP\n", getTime(), argv[0]);
        fclose(logFile);
		exit(1);
	}

		/* Now, all the initialization of the server is
		 * complete, and any user errors will have already
		 * been detected.  Now we can fork the daemon and
		 * return to the user.  We need to do a setpgrp
		 * so that the daemon will no longer be associated
		 * with the user's control terminal.  This is done
		 * before the fork, so that the child will not be
		 * a process group leader.  Otherwise, if the child
		 * were to open a terminal, it would become associated
		 * with that terminal as its control terminal.  It is
		 * always best for the parent to do the setpgrp.
		 */
	setpgrp();

	switch (fork()) 
	{
		case -1:		/* Unable to fork, for some reason. */
			perror(argv[0]);
			fprintf(logFile, "%s: %s: unable to fork daemon\n", getTime(), argv[0]);
		    fclose(logFile);
			exit(1);

		case 0:     /* The child process (daemon) comes here. */

				/* Close stdin and stderr so that they will not
				 * be kept open.  Stdout is assumed to have been
				 * redirected to some logging file, or /dev/null.
				 * From now on, the daemon will not report any
				 * error messages.  This daemon will loop forever,
				 * waiting for connections and forking a child
				 * server to handle each one.
				 */
			fclose(stdin);
			fclose(stderr);

				/* Set SIGCLD to SIG_IGN, in order to prevent
				 * the accumulation of zombies as each child
				 * terminates.  This means the daemon does not
				 * have to make wait calls to clean them up.
				 */
			if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
		        fprintf(logFile,"%s: %s: unable to register the SIGCHLD signal\n", getTime(), argv[0]);
		        fclose(logFile);
		        exit(1);
		        }
		        
			/* Register SIGTERM for an orderly server shutdown */
		    vec.sa_handler = (void *) finalizar;
		    vec.sa_flags = 0;
		    if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
		        fprintf(logFile,"%s: %s: unable to register the SIGTERM signal\n", getTime(), argv[0]);
		        fclose(logFile);
		        exit(1);
		        }

			while (!FIN) 
			{
		        /* Meter en el conjunto de sockets los sockets UDP y TCP */
		        FD_ZERO(&readmask);
		        FD_SET(ls_TCP, &readmask);
		        FD_SET(s_UDP, &readmask);
		        /* 
		        Seleccionar el descriptor del socket que ha cambiado. Deja una marca en 
		        el conjunto de sockets (readmask)
		        */ 
			    if (ls_TCP > s_UDP) s_mayor=ls_TCP;
				else s_mayor=s_UDP;

		        if ( (numfds = select(s_mayor+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0) {
		            if (errno == EINTR) {
		                FIN=1;
				        close (ls_TCP);
				        close (s_UDP);
		                fprintf(logFile, "%s: Interrupt signal in select\n", getTime()); 
		            }
		        }
		        else { 
		            /* Comprobamos si el socket seleccionado es el socket TCP */
		            if (FD_ISSET(ls_TCP, &readmask)) {
                            /* Note that addrlen is passed as a pointer
                            * so that the accept call can return the
                            * size of the returned address.
                            */
                            /* This call will block until a new
                            * connection arrives.  Then, it will
                            * return the address of the connecting
                            * peer, and a new socket descriptor, s,
                            * for that connection.
                            */
                        s_TCP = accept(ls_TCP, (struct sockaddr *) &clientaddr_in, &addrlen);
                        if (s_TCP == -1) exit(1);
                        switch (fork()) {
                            case -1:	/* Can't fork, just exit. */
                                exit(1);
                            case 0:		/* Child process comes here. */
                                close(ls_TCP); /* Close the listen socket inherited from the daemon. */
                                serverTCP(s_TCP, clientaddr_in);
                                exit(0);
                            default:	/* Daemon process comes here. */
                                    /* The daemon needs to remember
                                    * to close the new accept socket
                                    * after forking the child.  This
                                    * prevents the daemon from running
                                    * out of file descriptor space.  It
                                    * also means that when the server
                                    * closes the socket, that it will
                                    * allow the socket to be destroyed
                                    * since it will be the last close.
                                    */
                                close(s_TCP);
                        }
                    } /* De TCP*/
                    /* Comprobamos si el socket seleccionado es el socket UDP */
                    if (FD_ISSET(s_UDP, &readmask)) 
                    {
                            /* This call will block until a new
                            * request arrives.  Then, it will
                            * return the address of the client,
                            * and a buffer containing its request.
                            * BUFFERSIZE - 1 bytes are read so that
                            * room is left at the end of the buffer
                            * for a null character.
                            */
                            cc = recvfrom(s_UDP, buffer, BUFFERSIZE - 1, 0,
                            (struct sockaddr *)&clientaddr_in, &addrlen);

                            if ( cc == -1) 
                            {
                                fprintf(logFile, "%s: recvfrom error\n", argv[0]);
                                exit (1);
                            }

                            switch(fork())
                            {
                                case -1:	/* Can't fork, just exit. */
                                    exit(1);
                                case 0:		/* Child process comes here. */
                                    serverUDP (s_UDP, buffer, cc, clientaddr_in);
                                    exit(0);
                                default:
                                    continue;
                            }
                
                    }
                }
		    }

        close(ls_TCP);
        close(s_UDP);
    
        fprintf(logFile, "\nServer exiting...\n");
        fclose(logFile);

	default:
		exit(0);
	}

}

/*
 *				S E R V E R T C P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
void serverTCP(int s, struct sockaddr_in clientaddr_in)
{
	int reqcnt = 0;		/* keeps count of number of requests */
	char buf[TAM_BUFFER];		/* This example uses TAM_BUFFER byte messages. */
	char hostname[MAXHOST];		/* remote host's name string */

	int len, len1, status;
    struct hostent *hp;		/* pointer to host info for remote host */
    long timevar;			/* contains time returned by time() */
    
    struct linger linger;		/* allow a lingering, graceful close; */
    				            /* used when setting SO_LINGER */
    				
	/* Look up the host information for the remote host
	 * that we have connected with.  Its internet address
	 * was returned by the accept call, in the main
	 * daemon loop above.
	 */
	 
    status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0);
    if(status)
    {
    /* The information is unavailable for the remote
	* host.  Just format its internet address to be
	* printed out in the logging information.  The
	* address will be shown in "internet dot format".
	*/
	/* inet_ntop para interoperatividad con IPv6 */
    if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
        fprintf(logFile, "ERROR: inet_ntop \n");
    }

    /* Log a startup message. */
	fprintf(logFile, "%s: Startup from %s port %u.",
		    getTime(), hostname, ntohs(clientaddr_in.sin_port));

	/* Set the socket for a lingering, graceful close.
	* This will cause a final close of this socket to wait until all of the
	* data sent on it has been received by the remote host.
	*/
	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) 
    {
		errout(hostname);
	}

	/* Wait for a client request */
    packet package;
    if( !socket_receive(s, &package, NULL, 0, TCP)) return;

    if(package.opcode == RRQ)
        get_server(s, &package, NULL, 0, TCP);
    else if(package.opcode == WRQ)
        put_server(s, &package, NULL, 0, TCP);
    /* Illegal operation. Inform the client */
    else 
    {
		fprintf(logFile, "%s: Unexpected opcode value(TCP)\n", getTime());

		if( !build_ERR_packet(ERR_ILLEGAL_OP, "Illegal operation", &package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !socket_send(s, &package, NULL, 0, TCP) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }
	}

    close(s);

	fprintf(logFile, "%s: Completed %s port %u, %d requests\n", 
            getTime(), hostname, ntohs(clientaddr_in.sin_port), reqcnt);
}

/*
 *	This routine aborts the child process attending the client.
 */
void errout(char *hostname)
{
	fprintf(logFile, "Connection with %s aborted on error\n", hostname);
	exit(1);     
}


/*
 *	serverUDP
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client and process them.
 *
 */
void serverUDP(int s, char * buffer, size_t buffer_size, struct sockaddr_in clientaddr_in)
{
	int addrlen = sizeof(struct sockaddr_in);
    int udp_socket;
    struct sockaddr_in myaddr_in;

    memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));

    addrlen = sizeof(struct sockaddr_in);

	/* We create another socket to communicate with this specific client */
	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	myaddr_in.sin_port = 0;

    udp_socket = socket (AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
		fprintf(logFile, "%s: unable to create socket UDP\n", getTime());
        fclose(logFile);
		exit(1);
	   }

	if (bind(udp_socket, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		fprintf(logFile, "%s: unable to bind address UDP\n", getTime());
        fclose(logFile);
		exit(1);
	}

	/* Unserialize the first message received */
    packet package;
    unserialize((byte_t *)buffer, buffer_size, &package);
    
    if(package.opcode == RRQ)
        get_server(udp_socket, &package, &clientaddr_in, addrlen, UDP);
    else if(package.opcode == WRQ)
        put_server(udp_socket, &package, &clientaddr_in, addrlen, UDP);
    /* Illegal operation. Inform the client */
    else
    {
        fprintf(logFile, "%s: Unexpected opcode value(UDP)\n", getTime());
        if( !build_ERR_packet(ERR_ILLEGAL_OP, "Illegal operation", &package) )
        {           
            fprintf(logFile, "%s: Error building ERR packet\n", getTime());
            return;
        }

        if( !socket_send(udp_socket, &package, &clientaddr_in, addrlen, UDP) )
        {
            fprintf(logFile, "%s: Error sending ERR packet\n", getTime());
            return;
        }
    }

	close(udp_socket);
 }
