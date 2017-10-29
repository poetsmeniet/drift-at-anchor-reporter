#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tcp_client.h"
#include "irclib.h"
#define MAXLEN 200
#define RESPBUFSZ 5

//Call "list" and store all channels into channel list
extern int getAllChannels(int *clientSocket, chanList *chans){
    //Call list command
    printf("\n\n\nCalling 'list' - ");
    int rc = sendMessage(clientSocket, "list\n", 5);
    if(rc == 0)
        return 1;
    printf("rc = %d\n", rc);

    //Get all response data until socket times out
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));

    int testCnt = 0;

    while(1){
        int rc = recvMessage(clientSocket, responses, 1);

        //No response data after socket timeout
        if(rc == -1)
            return 1;

        //End of response data
        if(strstr(responses->buffer, "End of /LIST") != NULL)
            return 0;

        //Parse data line by line
        char *line = strtok(responses->buffer, "\n");
        while(line != NULL){
            printf("A line: %s\n", line);
            line = strtok(NULL, "\n");
        }
        testCnt++;
    }
    free(&responses->buffer[0]);
    free(responses);

    return 0;
}

extern int parseResponses(int *clientSocket){
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    
    while(1){
        int rc = recvMessage(clientSocket, responses, 1);

        //No response data after socket timeout
        if(rc == -1)
            continue;

        //Something whent horribly wrong..
        if(rc == -2){
            printf("TCP error?, quitting parseresponse\n");
            return -2;
        }

        //Echo to stdout
        printf("%s", responses->buffer);
        
        //Automatic ping
        if(strstr(responses->buffer, "PING") != NULL){
            printf("Replying to ping..");
            int rc = sendMessage(clientSocket, "pong\n", 5);
            if(rc == 0)
                return 1;
            printf("rc = %d\n", rc);
        }
        //Politeness
        if(strstr(responses->buffer, "ello") != NULL\
                || strstr(responses->buffer, "Hi") != NULL\
                || strstr(responses->buffer, "hi") != 0\
                ){
            printf("Being polite..");
            int rc = sendMessage(clientSocket, "PRIVMSG #geenbs :Hi!\n", 21);
            if(rc == 0)
                return 1;
            printf("rc = %d\n", rc);
        }
        responses->buffer[0] = '\0';
    }

    free(&responses->buffer[0]);
    free(responses);
    return 0;
}
    
//Join channels..
extern int joinChannels(int *clientSocket, chanList *chans){
    int rc;
    char *cmd = malloc(MAXLEN * sizeof(char));
    chanList *head = chans;

    while(head != NULL){
        printf("Joining channel: %s\n", head->chanName);

        snprintf(cmd, MAXLEN, "join #%s\n", head->chanName);

        rc = sendMessage(clientSocket, cmd, strlen(cmd));
        if(rc == 0)
            return 1;
        head = head->next;
    }
    free(cmd);
    return 0;
}

//Spawns interactive session to IRC server
//- mainly for debugging
extern int spawnShell(int *clientSocket){
    printf("Spawning shell..\n");
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
            printf("Replying to ping.. \n");
            rc = sendMessage(clientSocket, "pong\n", 5);
            if(rc == 0)
                return 1;
        }

        memset(cmd, 0, strlen(cmd));
        responses->buffer[0] = '\0';
    }

    free(cmd);
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
            free(req);

            if(rc == 0)
                return 1;
            
        }
        
        //End of login function success
        if(strstr(responses->buffer, " MODE ") != NULL)
            return 0;

        if(strstr(responses->buffer, "Nickname is already in us") != NULL)
            return 2;
        
    }

    free(responses);

    return 0;
}
