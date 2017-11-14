#ifndef generic_unix_tools_H_
#define generic_unix_tools_H_

/*A generic placeholder linked list, used by
 * freeLinkedListGen()*/
typedef struct agenericLinkedList{
    //int a;
    char chanName[100];
    struct agenericLinkedList *next;
}genL;

//Count nr of lines in file
extern int countLines(char *fileName);

/* Returns true or false if regex matches string
Usage: regexMatch("thomas$", "this is a test string mr thomas");*/
extern int regexMatch(char *regex, char *string);

//Generic linked list free
extern void freeLinkedListGen(void* targetList);
#endif
