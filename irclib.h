#ifndef irclib_H_
#define irclib_H_

//The roomList linked list stores 1 or more rooms
typedef struct roomList{
    char roomName[100];
    struct roomList *next;
}roomList;

//This struct stores initial data needed for login
//and room entry
typedef struct ircdata{
    char nick[100];
    char userName[100];
    roomList *rooms;
}ircData;

//logs into connected irc server using specified data
extern int ircLogin(ircData *ircData, int *clientSocket);

//Spawns interactive session to IRC server
//- mainly for debugging
extern int spawnShell(int *clientSocket);

//Join rooms..
extern int joinRooms(int *clientSocket, roomList *rooms);

#endif

