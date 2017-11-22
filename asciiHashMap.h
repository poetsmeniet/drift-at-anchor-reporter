/*
asciiHashMap V1

Copyright (c) 2017 Thomas Wink <thomas@geenbs.nl>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef asciiHashMap_H_
#define asciiHashMap_H_
#define MAXKEYSZ 50
#define ASCIISTART 32
#define ASCIIEND 127

//Key/value pairs, linked list
typedef struct hashMapEntry{
    char key[MAXKEYSZ]; 
    int value; //Int value, a counter. maps to 0 for getValue
    int replyNr; //This number relates to line cnt of replies. maps to 1
    struct hashMapEntry *next;
}hME;

/*Struct that contains hashmap entries
 * Seperated by first letter*/
typedef struct hashMap{
    char firstLetter;
    size_t totalCnt;
    hME *keys;
}hashMap;

//The hash is the first letter of key, linking to entries
extern int generateHashMap(hashMap *hMap);

/*addKey will attempt to add a new key/value pair;
 * The value is this example is a counter
 * if the key already exists, it will only increment the value*/
extern int addKey(hashMap *hMap, char *key, int replyNr, int len);

//Get the value of key in hashmap, -1 is not found
extern int getValue(hashMap *hMap, char *key, int replyNr, int returnVal);

//Just lists all entries in hashmap
extern void printHashMap(hashMap *hMap);

//Free the allocated memory
extern int freeHashMap(hashMap *hMap);
#endif
