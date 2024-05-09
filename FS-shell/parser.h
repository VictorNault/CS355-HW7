#ifndef PARSER_H
#define PARSER_H

char ** tokenize2(char * cmd, int * numCmds);
u_int8_t * chmodParsing(char * input, u_int8_t isDir, u_int8_t * curperms);
char * arrayToPermStr(u_int8_t * perms, int isDir);
#endif