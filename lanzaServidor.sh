#!/bin/bash

host=$1
serverID=$(pgrep server)

if [ -z $serverID ]; then
	echo "There is no server up"
else
	kill $serverID
fi

make clean && make server
./server
make client
./client $host UDP e testFile1.txt &
./client $host TCP e testFile2.txt &
./client $host UDP e testFile3.txt &
./client $host TCP e testFile4.txt &
./client $host UDP l testFile5.txt &
./client $host TCP l testFile6.txt &
./client $host UDP l testFile7.txt &
./client $host TCP l testFile8.txt &
