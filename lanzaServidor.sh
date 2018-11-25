!/bin/bash

serverID=$(pgrep server)

if [ -z $serverID ]; then
	echo "There is no server up"
else
	kill $serverID
fi

make clean &
make server &
make client

./server
sleep 1
./client localhost TCP l testFile.txt
