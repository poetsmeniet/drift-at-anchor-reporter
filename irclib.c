#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tcp_client.h"
#include "config.h"
#include "irclib.h"
#include "generic_unix_tools.h"
#define MAXLEN 200
#define RESPBUFSZ 5

extern int retrieveAutomatedReplies(aR *replies, char *fileName){
    FILE *fp;
    int lineNr = 0;
    int lineCnt = countLines(fileName);
    replies = malloc(lineCnt * sizeof(aR));
    printf("We have %d replies\n", lineCnt);
    fp = fopen(fileName, "r");

    if(fp != NULL){
        while(!feof(fp)){
            char regex[MAXLEN];
            char reply[MAXLEN];

            fscanf(fp, "%s %99[^\n]\n", &regex[0], reply);

            size_t keyLen = strlen(regex) + 1;
            size_t valLen = strlen(reply) + 1;
            if(keyLen > MAXLEN || valLen > MAXLEN){
               printf("\nSorry, maximum length of key value exceeded (%d)\n", MAXLEN);
               return 1;
            }

            //Check length of strings
            if(strlen(regex) > 0 && strlen(reply) > 0){
                printf("'%s' (%d) should result in reply '%s'(%d)\n", regex, strlen(regex), reply, strlen(reply));
                memcpy(replies[lineNr].regex, regex, strlen(regex));
                memcpy(replies[lineNr].reply, reply, strlen(reply));
                printf("'%s' stored result in reply '%s'\n", replies[lineNr].regex, replies[lineNr].reply);
            }
            lineNr++;

        }
    }else{
        printf("Unable to open '%s' for reading. Not loading automated responses\n", fileName);
    }
    fclose(fp);
    return 0;
}

//Call "list" and store all channels into channel list
extern int getAllChannels(int *clientSocket, chanList *chans){
    //Call list command
    int rc = sendMessage(clientSocket, "list\n", 5);

    if(rc == 0)
        return 1;
    
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));

    int chanCnt = 0;

    //Get all response data until socket times out
    while(1){
        int rc = recvMessage(clientSocket, responses, 1);
        
        //No response data after socket timeout
        if(rc == -1){
            free(responses);
            return 1;
        }

        char respCpy[MAXLEN];
        memcpy(respCpy, responses->buffer, strlen(responses->buffer));

        //Parse data line by line
        char *line = strtok(respCpy, "\n");
        while(line != NULL){
            //Tokenise using whitespace to extract channgel name
            char *token = strtok(line, " ");
            int tokCnt = 0;
            while(token != NULL){
                if(tokCnt == 3 && token[0] == '#'){

                    //Add to channel struct
                    size_t len = strlen(token);
                    printf("%d: Channel: %s, len = %d\n", chanCnt, token, len);
                    memcpy(chans->chanName, token, len); 
                    chans->chanName[len] = '\0';
                    chans->next = malloc(sizeof(chanList));;
                    chans->next->next = NULL;
                    chans = chans->next;

                    chanCnt++;
                }

                token = strtok(NULL, " ");
                tokCnt++;
            }
        
            //End of response data
            if(strstr(responses->buffer, "End of /LIST") != NULL){
                if(chanCnt == 0){
                    printf("Found %d channels..\n", chanCnt);
                    free(responses->buffer);
                    free(responses);
                    return -2;
                }
            }
            
            line = strtok(NULL, "\n");

            free(responses->buffer);
        }
    }

    return 0;
}

extern int parseResponses(int *clientSocket){
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    
    while(1){
        int rc = recvMessage(clientSocket, responses, 1);

        //No response data after socket timeout
        if(rc == -1){
            continue;
        }

        //Something whent horribly wrong..
        if(rc == -2){
            printf("TCP error?, quitting parseresponse\n");
            free(responses);
            free(responses->buffer);
            return -2;
        }

        //Echo to stdout
        printf("Server: %s", responses->buffer);
        
        //Automatic ping
        if(strstr(responses->buffer, "PING") != NULL){
            printf("\tReplying to ping..");
            int rc = sendMessage(clientSocket, "pong\n", 5);
            if(rc == 0){
                free(responses);
                free(responses->buffer);
                return 1;
            }
            printf("rc = %d\n", rc);
        }
        
        /*
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
        */
        responses->buffer[0] = '\0';
    }

    free(responses->buffer);
    free(responses);
    return 0;
}
    
//Join channels..
extern int joinChannels(int *clientSocket, chanList *chans){
    chanList *head = chans;

    if(head == NULL){
        printf("\tNo channels to join\n");
        return 1;
    }

    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    char cmd[86];
    int rc;

    while(head->next != NULL){
        printf("Joining channel: %s\n", head->chanName);

        snprintf(cmd, MAXLEN, "join %s\n", head->chanName);

        rc = sendMessage(clientSocket, cmd, strlen(cmd));
        if(rc == 0){
            free(responses);
            return 1;
        }
        
        while(recvMessage(clientSocket, responses, 1) != -1){
            printf("Server: %s", responses->buffer);

            //End of response data
            if(strstr(responses->buffer, "End of /NAMES list") != NULL){
                free(responses->buffer);
                break;
            }
            free(responses->buffer);
        }

        head = head->next;
    } 
    
    free(responses);
    return 0;
}

//Spawns interactive session to IRC server
//- mainly for debugging
extern int spawnShell(int *clientSocket){
    printf("Spawning shell..\n");

    //Loop stdin for issueing commands
    char cmd[86] = "Placeholder";
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    
    while(1){
        memset(cmd, 0, strlen(cmd));
        printf("> ");
        fgets(cmd, MAXLEN, stdin);
        
        int rc = sendMessage(clientSocket, cmd, strlen(cmd));
        if(rc == 0){
            free(responses);
            return 1;
        }
        
        recvMessage(clientSocket, responses, 1);
        printf("%s", responses->buffer);

        if(strstr(responses->buffer, "Quit") != NULL &&\
                strncmp(cmd, "quit", 4) == 0){
            free(responses->buffer);
            free(responses);
            return 0;
        }
        
        if(strstr(responses->buffer, "PING") != NULL){
            printf("Replying to ping.. \n");
            rc = sendMessage(clientSocket, "pong\n", 5);
            if(rc == 0){
                free(responses);
                free(responses->buffer);
                return 1;
            }
        }

        memset(cmd, 0, strlen(cmd));
        free(responses->buffer);
    }
    
    free(responses);
    free(responses->buffer);
    return 0;
}

//logs into connected irc server using specified data
extern int ircLogin(appConfig *ircData, int *clientSocket){
    //Send a message and get response(s) irc
    int rc;

    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));

    while(recvMessage(clientSocket, responses, 1)){
        printf("Server: %s", responses->buffer);
        
        //End of login function success
        if(strstr(responses->buffer, " MODE ") != NULL){
            free(responses->buffer);
            free(responses);
            return 0;
        }else if(strstr(responses->buffer, "Nickname is already in us") != NULL){
            printf("\tDetected NICK in use\n");
            free(responses->buffer);
            free(responses);
            return 2;
        }else if(strstr(responses->buffer, "Looking up") != NULL){
            //Respond to Checking Ident by sending login data
            printf("\tLogging in as '%s' len=%d..\n", ircData->nick, strlen(ircData->nick));
            char req[86];

            //Send Nick
            snprintf(req, MAXLEN, "NICK %s\n", ircData->nick);
            rc = sendMessage(clientSocket, req, strlen(req));
            if(rc == 0){
                free(responses->buffer);
                free(responses);
                return 1;
            }

            memset(req,0,strlen(req));

            //Send userName
            snprintf(req, MAXLEN, "USER %s 0 * :Test Bot\n", ircData->userName);
            rc = sendMessage(clientSocket, req, strlen(req));

            if(rc == 0){
                free(responses->buffer);
                free(responses);
                return 1;
            }
            free(responses->buffer);
        }else{
            //Nothing, just loop
            free(responses->buffer);
        }
    }

    return 0;
}
