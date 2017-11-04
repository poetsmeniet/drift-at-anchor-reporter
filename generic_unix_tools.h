#ifndef generic_unix_tools_H_
#define generic_unix_tools_H_

//Count nr of lines in file
extern int countLines(char *fileName);

/* Returns true or false if regex matches string
Usage: regexMatch("thomas$", "this is a test string mr thomas");*/
extern int regexMatch(char *regex, char *string);
#endif
