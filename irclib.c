#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "tcp_client.h"
#include "config.h"
#include "irclib.h"
#include "generic_unix_tools.h"
#define MAXLEN 200
#define RESPBUFSZ 5

//Returns user name from privmsg
int returnUserName(char *line, char *target){
    int len = strlen(line);
    int i;
    for(i = 1; i < len; i++){
        if(line[i] == '!' && line[i + 1] == '~'){
            target[i - 1] = '\0';
            return 1;
        }else{
            target[i - 1] = line[i];
        }
    }
    return 0;
}

//Copies token at specified index (channel or username)
int returnTokenAtIndex(char *line, int index, char *target){
    //Tokenise using whitespace
    char *token = strtok(line, " ");
    int tokCnt = 0;
    while(token != NULL){
        if(tokCnt == index && token[0] == '#'){
            //NULL is allowed as target
            if(target != NULL){
                //Copy to target
                size_t len = strlen(token);
                memcpy(target, token, len); 
                target[len] = '\0';
            }
            return 1;
        }
    
        token = strtok(NULL, " ");
        tokCnt++;
    }
    return 0;
}

/* Automated replies, triggered by regex
 * returns: nr of replies found in file */
extern int retrieveAutomatedReplies(aR *replies, char *fileName){
    FILE *fp;
    int lineNr = 0;
    int lineCnt = 0;
    fp = fopen(fileName, "r");

    if(fp != NULL){
        while(!feof(fp)){
            lineCnt++;
            int privateMsgFlag; //Private or channel msg
            int repeatMsgFlag; //Msg is to be repeated or not
            char regex[MAXLEN]; //Trigger
            char reply[MAXLEN]; //String reply

            int rc = fscanf(fp, "%d %d %s %99[^\n]\n", &privateMsgFlag, &repeatMsgFlag, &regex[0], reply);
            if(rc == 0){
                printf("\tError in '%s', line number %d\n", fileName, lineCnt);
                return -1;
            }

            size_t keyLen = strlen(regex);
            size_t valLen = strlen(reply);
            if(keyLen > MAXLEN || valLen > MAXLEN){
               printf("\nSorry, maximum length of key value exceeded (%d)\n", MAXLEN);
               return 0;
            }

            //Check length of strings and store
            // * temporarily added check for privateMsgFlag
            if(strlen(regex) > 0 && strlen(reply) > 0 && privateMsgFlag < 2){
                //Compile regex, case insensitive
                regex_t preg;
                int rc = regcomp(&preg, regex, REG_ICASE);
                if(rc != 0){
                    printf("\tRegex compilation failed, rc = %d (%s)\n", rc, regex);
                }else{
                    //Add reply parameters to struct
                    replies[lineNr].privateMsgFlag = privateMsgFlag;
                    replies[lineNr].repeatMsgFlag = repeatMsgFlag;
                    memcpy(replies[lineNr].regex, regex, strlen(regex));
                    replies[lineNr].regex[keyLen] = '\0';
                    memcpy(replies[lineNr].reply, reply, strlen(reply));
                    replies[lineNr].reply[valLen] = '\0';
                    lineNr++;

                    printf("\tAdded regex to replies '%s'\n", regex);
                }
            }
        }
    }else{
        printf("Unable to open '%s' for reading. Not loading automated responses\n", fileName);
        memcpy(replies[lineNr].regex, "EOA\0", 4);
        return 0;
    }

    //Terminate this array
    memcpy(replies[lineNr].regex, "EOA\0", 4);
    fclose(fp);

    return lineNr;;
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

            int rc2 = returnTokenAtIndex(line, 3, chans->chanName);
            if(rc2 == 1){
                chans->next = malloc(sizeof(chanList));;
                chans->next->next = NULL;
                chans = chans->next;
    
                chanCnt++;
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
        }
    }

    free(responses->buffer);
    printf("here2...\n");

    return 0;
}

extern int parseResponses(int *clientSocket, aR *replies){
    respBuf *responses = malloc(RESPBUFSZ * sizeof(respBuf));
    
    while(1){
        int rc = recvMessage(clientSocket, responses, 1);

        if(rc == -1) //No response data after socket timeout
            continue;

        if(rc == -2){//Something whent horribly wrong..
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
        
        //Check all automated responses and reply accordingly
        int cnt = 0;
        while(bcmp(replies[cnt].regex, "EOA\0", 4) != 0){
            if(regexMatch(replies[cnt].regex, responses->buffer) == 0){

                //Copy responses buffer
                char *respCpy = malloc(strlen(responses->buffer) * sizeof(char));
                memcpy(respCpy, responses->buffer, strlen(responses->buffer));

                //Retrieve channale name
                char respChan[100];
                int rc2 = returnTokenAtIndex(respCpy, 2, respChan);

                //Get user name to respons to
                char respUsr[100];
                returnUserName(responses->buffer, respUsr);

                //Compose the private message to user
                char thisReply[200] = "";
                if(rc2 == 0 && replies[cnt].privateMsgFlag == 1){
                    printf("\tUser to reply to: '%s'\n", respUsr);
                    sprintf(thisReply, "PRIVMSG %s :%s\n", respUsr, replies[cnt].reply);
                }
                
                //Compose channel message
                if(rc2 == 1 && replies[cnt].privateMsgFlag == 0){
                    sprintf(thisReply, "PRIVMSG %s :%s\n", respChan, replies[cnt].reply);
                }

                //Send message
                if(strlen(thisReply) > 0){
                    printf("\nComposed response: '%s' len=%d\n\n", thisReply, strlen(thisReply));
                    int rc = sendMessage(clientSocket, thisReply, strlen(thisReply));

                    if(rc == 0){
                        printf("do some error handling dude\n");
                    }
                }
            }
            cnt++;
        }
        
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

//Free channels linked list
extern void freeChannels(chanList *targetList){
    //Onlye one element, otherwise free consecutive elements
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
    printf("Done freeing channels\n");
}
