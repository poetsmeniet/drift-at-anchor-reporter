#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcp_client.h"
#include "config.h"
#include "irclib.h"
#include "generic_unix_tools.h"

int main(void){
    printf("Retrieving automated responses...\n");
    //todo: Parameterize this filename
    int lineCnt = countLines("replies.txt");
    aR replies[lineCnt + 1];
    if(retrieveAutomatedReplies(replies, "replies.txt") == -1){
        printf("There is an error in your replies configuration\n");
        return 1;
    }
    
    
    //Retreive configuration parameters
    appConfig config;
    if(getConfig(&config, "config.txt") == 1){ //Offload this later as parameter
        printf("Loading of config failed, file '%s'\n", "config.txt");
        return 1;
    }

    //Connect to server and retrieve socket
    int clientSocket = connectToServer(config.serverName, config.serverPort, 12);

    if(clientSocket == 0){
        printf("Connect failed with code %d\n", clientSocket);
        return 1;
    }

    int rc = ircLogin(&config, &clientSocket);

    if(rc > 0){
        printf("Error logging in, rc %d\n", rc);
        return 1;
    }
    
    //test channels linked list, todo: offload this to external file
    chanList *chans = malloc(sizeof(chanList));
    chans->next = NULL;

    printf("\nRequesting all channels...\n");
    if(getAllChannels(&clientSocket, chans) == -2){
        printf("Recall getallchans..\n");
        rc = getAllChannels(&clientSocket, chans);
    }
    if(rc == 0)
        rc = joinChannels(&clientSocket, chans);

    //Running parsing of responses
    printf("Starting parseResponses..\n");
    parseResponses(&clientSocket, replies);
    
    //Spawn "shell", mainly for debugging
    //spawnShell(&clientSocket);

    close(clientSocket);

    freeChannels(chans);

    return 0;
}

