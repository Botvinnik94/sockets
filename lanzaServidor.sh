#!/bin/bash

host=$1

rm cliente.txt
rm server.txt

make clean
make server
./server
make client
./client $host TCP e testFile1.txt &
./client $host UDP e testFile2.txt &
./client $host TCP e testFile3.txt &
./client $host UDP e testFile4.txt &
./client $host TCP l testFile5.txt &
./client $host UDP l testFile6.txt &
./client $host TCP l testFile7.txt &
./client $host UDP l testFile8.txt &

sleep 2
serverID=$(pgrep server)
if [ -z $serverID ]; then
	echo "There is no server up"
else
	kill $serverID
fi
