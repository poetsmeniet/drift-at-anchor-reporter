#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tcp_client.h"
#include "irclib.h"
#define MAXLEN 200

//logs into connected irc server using specified data
extern int ircLogin(ircData *ircData, int *clientSocket){
    //Send a message and get response(s) irc
    int rc;


    respBuf *responses = malloc(5 * sizeof(respBuf));
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
    }

    return 0;
}
