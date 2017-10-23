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
    //char serverName[] = "irc.freenode.org";
    //int serverPort =  6667;
    int clientSocket = connectToServer(serverName, serverPort, 20);

    if(clientSocket == 0){
        printf("Connect failed with code %d\n", clientSocket);
        return 1;
    }

    ircData myData;
    strncpy(myData.nick, "Abot\0", 5);
    strncpy(myData.userName, "Bot Name\0", 10);

    int rc = ircLogin(&myData, &clientSocket);

    if(rc > 0){
        printf("Error logging in, rc %d\n", rc);
        return 1;
    }
    




    ////Loop stdin for issueing commands
    //char *cmd = malloc(86 * sizeof(char));
    //while(1){
    //    memset(cmd, 0, strlen(cmd));
    //    printf("> ");
    //    fgets(cmd, 86, stdin);
    //    printf("issuing comand '%s'\n", cmd);
    //    
    //    
    //    respCnt = 0;
    //    rc = sendMessage(&clientSocket, cmd, strlen(cmd), respCnt, responses);

    //    sleep(1);
    //    
    //    memset(cmd, 0, strlen(cmd));
    //    respCnt = 1;
    //    rc = sendMessage(&clientSocket, cmd, 0, respCnt, responses);
    //    
    //    //Print response(s)
    //    if(rc > 0){
    //        for(i = 0; i < respCnt; i++)
    //            printf("%s", responses[i].buffer);
    //    }
    //}
    
    //free(responses);
    //free(req);

    //closeSocket();

    return 0;
}
