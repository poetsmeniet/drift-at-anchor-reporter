#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tcp_client.h"
#include "irclib.h"
#define MAXLEN 200
#define RESPBUFSZ 5

extern int parseResponses(int *clientSocket){
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    
    while(1){
        recvMessage(clientSocket, responses, 1);
        printf("%s", responses->buffer);
        
        if(strstr(responses->buffer, "PING") != NULL){
            printf("replying to ping..\n");
            if(sendMessage(clientSocket, "pong", 4) == 0)
                return 1;
        }
        responses->buffer[0] = '\0';
    }

    return 0;
}
    
//Join channels..
extern int joinChannels(int *clientSocket, chanList *chans){
    chanList *head = chans;
    while(head != NULL){
        printf("Joining channel: %s\n", head->chanName);

        char *cmd = malloc(MAXLEN * sizeof(char));
        snprintf(cmd, MAXLEN, "join #%s\n", head->chanName);

        int rc = sendMessage(clientSocket, cmd, strlen(cmd));
        if(rc == 0)
            return 1;
        head = head->next;
    }

    return 0;
}

//Spawns interactive session to IRC server
//- mainly for debugging
extern int spawnShell(int *clientSocket){
    //Loop stdin for issueing commands
    char *cmd = malloc(86 * sizeof(char));
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    
    while(1){
        memset(cmd, 0, strlen(cmd));
        printf("> ");
        fgets(cmd, MAXLEN, stdin);
        
        int rc = sendMessage(clientSocket, cmd, strlen(cmd));
        if(rc == 0)
            return 1;
        
        recvMessage(clientSocket, responses, 1);
        printf("%s", responses->buffer);

        if(strstr(responses->buffer, "Quit") != NULL &&\
                strncmp(cmd, "quit", 4) == 0)
            return 0;
        
        if(strstr(responses->buffer, "PING") != NULL){
            printf("Replying to ping..\n");
            rc = sendMessage(clientSocket, "pong", 4);
            if(rc == 0)
                return 1;
        }

        memset(cmd, 0, strlen(cmd));
        responses->buffer[0] = '\0';
    }

    return 0;
}

//logs into connected irc server using specified data
extern int ircLogin(ircData *ircData, int *clientSocket){
    //Send a message and get response(s) irc
    int rc;

    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));

    while(recvMessage(clientSocket, responses, 1)){
        printf("Server says %s", responses->buffer);

        //Respond to Checking Ident by sending login data
        if(strstr(responses->buffer, "Checking Ident") != NULL){
            printf("Logging in as '%s' len=%d..\n", ircData->nick, strlen(ircData->nick));
            char *req = malloc(MAXLEN * sizeof(char));

            //Send Nick
            snprintf(req, MAXLEN, "NICK %s\n", ircData->nick);
            rc = sendMessage(clientSocket, req, strlen(req));
            if(rc == 0)
                return 1;
            memset(req,0,strlen(req));

            //Send userName
            snprintf(req, MAXLEN, "USER %s 0 * :Test Bot\n", ircData->userName);
            rc = sendMessage(clientSocket, req, strlen(req));
            if(rc == 0)
                return 1;
            memset(req,0,strlen(req));
        }
        
        //End of login function success
        if(strstr(responses->buffer, " MODE ") != NULL)
            return 0;

        if(strstr(responses->buffer, "Nickname is already in us") != NULL)
            return 2;
        
        responses->buffer[0] = '\0';
    }

    return 0;
}
