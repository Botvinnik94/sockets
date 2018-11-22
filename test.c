#include <stdio.h>
#include "packet.h"

int main()
{
    byte_t buffer[15];

    
    packet pkg;

    buffer[0] = EMPTY;
    buffer[1] = DATA;/*
    int i;
    for(i = 2; i < 7; ++i){
        buffer[i] = 'a';
    }
    buffer[i++] = '\0';
    for(;i < 14; i++){
        buffer[i] = 'b';
    }
    buffer[i] = '\0';
    */

   buffer[2] = EMPTY;
   buffer[3] = 0b0010000;

    int i;
    for(i = 4; i < 15; i++){
        buffer[i] = 'a';
    }

   unserialize(buffer, 15, &pkg);

   printf("\n\n%hu %c%c%c", pkg.data_message.nBloq, pkg.data_message.data[1], pkg.data_message.data[4], pkg.data_message.data[6]);

   
}