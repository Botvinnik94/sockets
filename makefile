client: client.o action.o transfer.o packet.o
	gcc -g client.o action.o transfer.o packet.o -o client

server: server.o action.o transfer.o packet.o
	gcc -g server.o action.o transfer.o packet.o -o server

client.o: client.c
	gcc -c -g client.c

action.o: action.c action.h
	gcc -c -g action.c

transfer.o: transfer.c transfer.h
	gcc -c -g transfer.c

packet.o: packet.c packet.h
	gcc -c -g packet.c

server.o: server.c
	gcc -c -g server.c

clean:
	rm *.o client server testFile.txt cliente.txt

