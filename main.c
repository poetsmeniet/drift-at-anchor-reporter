#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcp_client.h"
#include "irclib.h"

int main(void){

    //Connect to server
    char serverName[] = "geenbs.nl";
    int serverPort =  6665;
    int clientSocket = connectToServer(serverName, serverPort, 12);

    if(clientSocket == 0){
        printf("Connect failed with code %d\n", clientSocket);
        return 1;
    }

    //Populate ircData struct
    ircData myData;
    strncpy(myData.nick, "Abot9\0", 6);
    strncpy(myData.userName, "Bot Name\0", 10);

    int rc = ircLogin(&myData, &clientSocket);

    if(rc > 0){
        printf("Error logging in, rc %d\n", rc);
        return 1;
    }
    
    //test rooms linked list, todo: offload this to external file
    roomList *rooms = malloc(sizeof(roomList));
    memcpy(rooms->roomName, "geenbs\0", 7); 
    rooms->next = malloc(sizeof(roomList));;
    memcpy(rooms->next->roomName, "botroom\0", 8); 
    rooms->next->next = '\0';

    joinRooms(&clientSocket, rooms);

    //readIrc
    
    spawnShell(&clientSocket);

    close(clientSocket);

    return 0;
}
