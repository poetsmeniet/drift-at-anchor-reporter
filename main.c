#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcp_client.h"
#include "irclib.h"

int main(void){

    //Connect to server
    //char serverName[] = "geenbs.nl";
    //int serverPort =  6665;
    char serverName[] = "irc.freenode.org";
    int serverPort =  6667;
    int clientSocket = connectToServer(serverName, serverPort, 12);

    if(clientSocket == 0){
        printf("Connect failed with code %d\n", clientSocket);
        return 1;
    }
    printf("Socket has nr %d\n", clientSocket);

    //Populate ircData struct
    ircData myData;
    strncpy(myData.nick, "Abot9\0", 6);
    strncpy(myData.userName, "Bot Name\0", 10);

    int rc = ircLogin(&myData, &clientSocket);

    if(rc > 0){
        printf("Error logging in, rc %d\n", rc);
        return 1;
    }
    
    //test channels linked list, todo: offload this to external file
    chanList *chans = malloc(sizeof(chanList));
    memcpy(chans->chanName, "geenbs\0", 7); 
    chans->next = malloc(sizeof(chanList));;
    memcpy(chans->next->chanName, "botroom\0", 8); 
    chans->next->next = '\0';

    getAllChannels(&clientSocket, chans);

    //joinChannels(&clientSocket, chans);

    //parseResponses(&clientSocket);
    
    spawnShell(&clientSocket);

    close(clientSocket);

    return 0;
}
