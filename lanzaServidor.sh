#!/bin/bash

serverID=$(pgrep server)
#kill $serverID 2> /dev/null
if [ -z $serverID ]; then
	echo "There is no server up"
else
	kill $serverID
fi

rm cliente.txt
rm server.txt
#rm ficherosTFTPserver/testFile.txt

make clean
sleep 1.5
make server
sleep 1.5
make client
sleep 1.5
./server
sleep 1
./client localhost UDP e testFile.txt
#./client localhost TCP l testFile1.txt
