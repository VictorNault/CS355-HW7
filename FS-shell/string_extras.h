#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef STRING_EXTRAS_H
#define STRING_EXTRAS_H


char * trimStr(char * str);
int countChar(char * str,char target);
int inStr(char * str, char * chars);
#endif