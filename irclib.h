#ifndef irclib_H_
#define irclib_H_
#define MAXREGEXSZ 200
#define MAXREPLYSZ 500

//The chanList linked list stores 1 or more channels
typedef struct chanList{
    char chanName[100];
    struct chanList *next;
}chanList;

typedef struct automaticReplies{
    char regex[MAXREGEXSZ]; //This regex match will trigger reply
    char reply[MAXREPLYSZ]; //Reply body
}aR;

//DEPRE:
//This struct stores initial data needed for login
//and channel entry
typedef struct ircdata{
    chanList *chans;
}ircData;

//logs into connected irc server using specified data
extern int ircLogin(appConfig *ircData, int *clientSocket);

//Spawns interactive session to IRC server
//- mainly for debugging
extern int spawnShell(int *clientSocket);

//Join channels..
extern int joinChannels(int *clientSocket, chanList *chans);

//Parse respones loop, add your own code here
extern int parseResponses(int *clientSocket);

//Call "list" and store all channels into channel list
extern int getAllChannels(int *clientSocket, chanList *chans);

//Retrieves all automated responses, including regex triggers
extern int retrieveAutomatedReplies(aR *replies, char *fileName);
#endif
