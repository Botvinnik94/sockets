#!/bin/bash

serverID=$(pgrep server)

if [ -z $serverID ]; then
	echo "There is no server up"
else
	kill $serverID
fi

make clean
sleep 1.5
make server
sleep 1.5
make client

./server
sleep 1
./client localhost UDP l testFile1.txt 
./client localhost UDP l testFile2.txt
./client localhost UDP l testFile3.txt
./client localhost UDP l testFile4.txt
