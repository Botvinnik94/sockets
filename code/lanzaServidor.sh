#!/bin/bash

./server

sleep 1

./client localhost TCP l testFile5.txt &
./client localhost UDP l testFile6.txt &
./client localhost TCP l testFile7.txt &
./client localhost UDP l testFile8.txt &
./client localhost TCP e testFile1.txt &
./client localhost UDP e testFile2.txt &
./client localhost TCP e testFile3.txt &
./client localhost UDP e testFile4.txt &
