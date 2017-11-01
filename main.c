#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcp_client.h"
#include "config.h"
#include "irclib.h"

void freeLinkedList(chanList *targetList){
    if(targetList->next == NULL){
        free(targetList);
    }else{
        chanList *head = targetList;
        chanList *curr;
        while ((curr = head) != NULL) { // set curr to head, stop if list empty.
            head = head->next;          // advance head to next element.
            free (curr);                // delete saved pointer.
        }
    }
    printf("Done freeing targetList\n");
}

int main(void){

    appConfig config;
    getConfig(&config, "config.txt"); //Offload this later as parameter

    //Connect to server
    //char serverName[] = "germany.enterthegame.com";
    //int serverPort =  6665;
    //char serverName[] = "geenbs.nl";
    //int serverPort =  6665;
    //char serverName[] = "irc.freenode.org";
    //int serverPort =  6667;
    int clientSocket = connectToServer(config.serverName, config.serverPort, 12);

    if(clientSocket == 0){
        printf("Connect failed with code %d\n", clientSocket);
        return 1;
    }

    int rc = ircLogin(&config, &clientSocket);

    printf("irclogin rc = %d\n", rc);
    if(rc > 0){
        printf("Error logging in, rc %d\n", rc);
        return 1;
    }
    
    //test channels linked list, todo: offload this to external file
    chanList *chans = malloc(sizeof(chanList));

    printf("\nRequesting channel list - ");
    if(getAllChannels(&clientSocket, chans) == -2){
        printf("Recall getallchans..\n");
        getAllChannels(&clientSocket, chans);
    }

    rc = joinChannels(&clientSocket, chans);

    printf("Starting parseResponses..\n");
    parseResponses(&clientSocket);
    
    //spawnShell(&clientSocket);

    close(clientSocket);

    if(chans->next != NULL)
        freeLinkedList(chans);
    else
        free(chans);

    return 0;
}
