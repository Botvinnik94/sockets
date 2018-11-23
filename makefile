clientcp: clientcp.o action.o transfer.o packet.o
	gcc clientcp.o action.o transfer.o packet.o -o clientcp

clientcp.o: clientcp.c
	gcc -c clientcp.c

action.o: action.c action.h
	gcc -c action.c

transfer.o: transfer.c transfer.h
	gcc -c transfer.c

packet.o: packet.c packet.h
	gcc -c packet.c

clean:
	rm *.o clientcp

