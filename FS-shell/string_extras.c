#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Accepts a str as a parameter removes leading and trailing white space as a malloced copy.
*/
char * trimStr(char * str){
    int len = strlen(str)-1;
    int inc = 0;
    char * strp = str;
    while(isspace(str[len]) && len >= 0){
        len--;
    }
    if (len == -1) return "";

    while(*strp && isspace(*strp)){
        inc++;
        strp++;
    }
    char * str_copy =  (char *) malloc (sizeof(char) * ((len+1)-inc)+1);
    memcpy(str_copy,strp,((len+1)-inc));
    str_copy[len-inc+1] = '\0';
    return str_copy;
}

int countChar(char * str,char target){
    int count = 0;
    for (int i = 0; i < strlen(str); i++){
        if (str[i] == target)count++;
    }
    return count;
}

int inStr(char * str, char * chars){
    int num;
    for (int i = 0; i < strlen(chars);i++){
    num = countChar(str,chars[i]);
        if (num > 0){
            return 1;
        }
    }
    return 0;

}
