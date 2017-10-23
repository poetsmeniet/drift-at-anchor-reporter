#ifndef irclib_H_
#define irclib_H_

//The roomList linked list stores 1 or more rooms
typedef struct roomsList{
    char roomName[100];
    struct roomsList *next;
}roomsList;

//This struct stores initial data needed for login
//and room entry
typedef struct ircdata{
    char nick[100];
    char userName[100];
    roomsList *rooms;
}ircData;

//logs into connected irc server using specified data
extern int ircLogin(ircData *ircData, int *clientSocket);

#endif

