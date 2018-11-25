#!/bin/bash

serverID=$(pgrep server)
if [-z $serverID]; then
	echo "There is no server up"
else
	kill $serverID
fi

rm cliente.txt
rm server.txt
rm ficherosTFTPserver/testFile.txt

make server &
make client &
make clean

./server
sleep 1
./client localhost TCP l testFile.txt

