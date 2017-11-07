#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcp_client.h"
#include "config.h"
#include "irclib.h"
#include "generic_unix_tools.h"

extern void freeLinkedList(chanList *targetList);

int main(void){
    printf("Retrieving automated responses...\n");
    
    //Parameterize this filename
    int lineCnt = countLines("replies.txt");
    aR replies[lineCnt + 1];
    retrieveAutomatedReplies(replies, "replies.txt");

    //Retreive configuration parameters
    appConfig config;
    if(getConfig(&config, "config.txt") == 1){ //Offload this later as parameter
        printf("Loading of config failed, file '%s'\n", "config.txt");
        return 1;
    }

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

    printf("\nRequesting all channels...\n");
    if(getAllChannels(&clientSocket, chans) == -2){
        printf("Recall getallchans..\n");
        rc = getAllChannels(&clientSocket, chans);
    }
    if(rc == 0)
        rc = joinChannels(&clientSocket, chans);

    printf("Starting parseResponses..\n");
    parseResponses(&clientSocket, replies);
    
    //spawnShell(&clientSocket);

    close(clientSocket);

    if(chans->next != NULL)
        freeLinkedList(chans);
    else
        free(chans);

    //Dont forget to free replies

    return 0;
}

//Free a linked list
extern void freeLinkedList(chanList *targetList){
    
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
