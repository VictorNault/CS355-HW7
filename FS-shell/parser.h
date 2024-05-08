#ifndef PARSER_H
#define PARSER_H

char ** tokenize2(char * cmd, int * numCmds);
int * chmodParsing(char * input, int isDir, int * curperms);
char * arrayToPermStr(u_int8_t * perms, int isDir);
#endif