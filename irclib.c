#include <stdio.h>
#include "tcp_client.h"
#include "irclib.h"

//logs into connected irc server using specified data
extern int irclogin(ircData *ircData, int *clientSocket){
    //Send a message and get response(s) irc
    int rc;
    char *req = malloc(200 * sizeof(char));
    memcpy(req, "NICK Testbot\n", 13);

    rc = sendMessage(&clientSocket, req, strlen(req));
    if(rc == 0){
        printf("Sending messsage 1 failed.\n");
        return 1;
    }
    memset(req,0,strlen(req));
    return 1;
}
