#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <sys/types.h>
#include "generic_unix_tools.h"

/*Free a linked list
 * Will work if the "genL" struct type matches your struct in size 
 * Customize if needed or adjust genL struct */
extern void freeLinkedListGen(void* targetList){
    genL *tL = (genL *)targetList; //Explicit

    if(tL->next == NULL){ //Only one node, the first
        free(tL);
    }else{
        genL *head = tL;
        genL *curr;
        while ((curr = head) != NULL) { //Set curr to head, stop if list empty.
            head = head->next;          //Advance head to next element.
            free (curr);                //Free saved pointer.
        }
    }
}

//Count nr of lines in file
extern int countLines(char *fileName){
    FILE *fp;
    int lineCnt = 0;
    fp = fopen(fileName, "r");

    if(fp != NULL){
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            lineCnt++;
        }
        fclose(fp);
    }
    return lineCnt;
}

/* Returns true or false if regex matches string
Usage: regexMatch("thomas$", "this is a test string mr thomas");*/
extern int regexMatch(char *regex, char *string){
    //Check lengths
    if(strlen(regex) == 0 || strlen(string) == 0){
        printf("Regex or string empty\n");
        return -1;
    }
    regex_t preg;

    //Compile regex, case insensitive
    int rc = regcomp(&preg, regex, REG_ICASE);
    if(rc != 0){
        printf("Regex compilation failed, rc = %d\n", rc);
        return 1;
    }

    int nmatch = 0;
    regmatch_t *pmatch = NULL;
    rc = regexec(&preg, string, nmatch, pmatch, 0);

    if(rc == 0){
        //printf("Match found, regexec returns %d, nmatch: %d\n", rc, nmatch);
        return 0;
    }else{
        //printf("Match not found, regexec returns %d, nmatch: %d\n", rc, nmatch);
        return 1;
    }
    return 1;
}
