#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "string_extras.h"
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>
#define TRUE 1
#define FALSE 0
// char ** splitSemiColon(char * stringToSplit, int * numCmds){
    // char * stringToSplitCopy = malloc( sizeof(char) * (strlen(stringToSplit)+1));
    // strcpy(stringToSplitCopy, stringToSplit); 
    // int count = 0; // counting & and ; to detemrine how many commands we have
    // for (int i = 0; i < strlen(stringToSplitCopy); i++){
    //     if (stringToSplitCopy[i] == '&' || stringToSplitCopy[i] == ';') count++;
    // }
    // if (count == 0) count ++; // no & or ; still a command
    // int index = 0;
    // *numCmds = count+1;
//     char * prev_ptr = stringToSplitCopy;
//     int token_index = 0;
    // char ** tokens = malloc(sizeof(char *) * count);
//     while(stringToSplitCopy[index] != '\0'){
//         if (stringToSplitCopy[index] == '&'){
//             stringToSplitCopy[index] = '\0';
//             int stringLength = strlen(stringToSplitCopy) + 2;
//             tokens[token_index] = malloc(sizeof(char) * stringLength);// +1 \0 +1 \b 
            // strcpy(tokens[token_index],(const char *) prev_ptr);
//             tokens[token_index][stringLength-1] = '\0';
//             tokens[token_index][stringLength-2] = '9'; // replace with \a later
//             printf("%s\n",tokens[token_index]);

//             token_index++;

//             prev_ptr = &stringToSplitCopy[index+1];
//         }
//         else if (stringToSplitCopy[index] == ';'){
//             stringToSplitCopy[index] = '\0';
//             int stringLength = strlen(stringToSplitCopy) + 1;
//             tokens[token_index] = malloc(sizeof(char) * stringLength);// +1 \0 +1 \b 
//             strcpy(tokens[token_index],(const char *) prev_ptr);
//             tokens[token_index][stringLength-1] = '\0';
//             token_index++;
//             prev_ptr = &stringToSplitCopy[index+1];
            

//         }
//         index++;
//     }
// if (prev_ptr[0] != '\0'){
    // stringToSplitCopy[index] = '\0';
    // int stringLength = strlen(stringToSplitCopy) + 1;
    // tokens[token_index] = malloc(sizeof(char) * stringLength);// +1 \0 
    // strcpy(tokens[token_index],(const char *) prev_ptr);
    // tokens[token_index][stringLength-1] = '\0';
// }
//     return tokens;
// }

char ** tokenize2(char * cmd, int * numCmds){
    if(strcmp(cmd,"") == 0){
        *numCmds = 0;
        return NULL;
    }

    char * cmdCopy =trimStr(cmd);
    // printf("%s\n",cmdCopy);

    int count = 1; // counting & and ; to detemrine how many commands we have
    for (int i = 0; i < strlen(cmdCopy); i++){
        if (cmdCopy[i] == '&' || cmdCopy[i] == ';') count++;
    }

    if (cmdCopy[strlen(cmdCopy)-1] == '&' || cmdCopy[strlen(cmdCopy)-1] == ';') count--;

    int index = 0;
    *numCmds = count;
    char ** tokens = malloc(sizeof(char *) * count);
    int tok_idx = 0;
    char * prev_ptr = cmdCopy;
    char * search = strpbrk(cmdCopy,"&;");
    while(search != NULL){
        // printf("%s", search);
        if (*search == '&'){
            *search = '\0';
            int tok_size = strlen(prev_ptr)+2;
            tokens[tok_idx] = malloc(sizeof(char) * tok_size);
            strcpy(tokens[tok_idx],(const char *) prev_ptr);

            tokens[tok_idx][tok_size - 2] = '&'; // temporary B will replace with \a later (bell)
            tokens[tok_idx][tok_size-1] = '\0';
            prev_ptr = search + 1;
            search = search + 1; 
        }
        else{
            *search = '\0';
            int tok_size = strlen(prev_ptr)+1;
            tokens[tok_idx] = malloc(sizeof(char) * tok_size);
            strcpy(tokens[tok_idx],(const char *) prev_ptr);
            tokens[tok_idx][tok_size-1] = '\0'; 
            prev_ptr = search + 1;
            search = search + 1; 
        }   
        tok_idx++;     
        search = strpbrk(search, "&;");
    }

    if(*prev_ptr != '\0' || prev_ptr == cmdCopy){
        tokens[tok_idx] = '\0';
        int stringLength = strlen(prev_ptr) + 1;
        tokens[tok_idx] = malloc(sizeof(char) * stringLength);// +1 \0 &;",
        strcpy(tokens[tok_idx],(const char *) prev_ptr);

    }
    free(cmdCopy);
    return tokens;
}

char * arrayToPermStr(u_int8_t * perms, int isDir){
    char * output = malloc(sizeof(char)*11);
    output[0] = (isDir == 1) ? 'd' : '-';
    for (int i = 0; i < 3; i++){
        output[1+i*3] = (perms[i*3] == 1) ? 'r' : '-';
        output[1+i*3+1] = (perms[i*3+1] == 1) ? 'w' : '-';
        output[1+i*3+2] = (perms[i*3+2] == 1) ? 'x' : '-';
    }
    output[10] = '\0';
    return output;
}

u_int8_t * chmodParsing(char * input, u_int8_t isDir, u_int8_t * curperms){
    //TODO : split based on +-= so group must follow perms;
    // starting with octal
    u_int8_t * output = curperms;
    output[0] = isDir % 1;
    u_int8_t isOctal = 1;
    u_int8_t len = strlen(input);
    if (len == 3){
        for (u_int8_t i = 0; i < strlen(input); i++){
            if (isdigit(input[i]) != 0 && (u_int8_t)(input[i]-'0') < 8){
                continue;
            }
            else{
                isOctal = -1;
            }
        }
    }
    else isOctal = -1;
    if (isOctal == 1){ // octal processing
        u_int8_t owner = (input[0]-'0');
        u_int8_t group = (input[1]-'0');
        u_int8_t all = input[2]-'0';
        u_int8_t perms[3] = {owner,group,all};
        u_int8_t octal = owner*100+group*10 + all;
        for (int i = 0; i < 3; i++){
            // r
            if ((perms[i] & 4) != 0){
                output[1+i*3] = 1;
            }
            //w
            if ((perms[i] & 2) != 0){
                output[1+i*3+1] = 1;
            }
            //x
            if ((perms[i] & 1) != 0 ){
                output[1+i*3+2] = 1;
            }
        } 
        return curperms;
    }
    else{ // symbolic processing
        u_int8_t perms[3] = {0};
        // finding which groups are being updated; 
        u_int8_t num_eq = countChar(input, '=');
        u_int8_t num_p = countChar(input, '+');
        u_int8_t num_s = countChar(input,'-');
        if (num_eq+num_p+num_s != 1){
            printf("Invalid mode '%s'\n", input);
            return NULL;
        }
        if(inStr(input,"=+-") != 1){
            printf("Invalid mode '%s'\n", input);
        }
        printf("%d",inStr(input,"o"));
        if (inStr(input,"u") == 1) perms[0] = 1;
                
        if (inStr(input,"g") == 1) perms[1] = 1;
                
        if (inStr(input,"o") == 1) perms[2] = 1;

        if (inStr(input,"a") == 1){
            perms[0] = 1;
            perms[1] = 1;
            perms[2] = 1;
        }

        int rwx[3] = {0,0,0};

        if (inStr(input,"r") == 1) rwx[0] = 1;
                
        if (inStr(input,"w") == 1) rwx[1] = 1;
                
        if (inStr(input,"x") == 1) rwx[2] = 1;

        if(num_eq == 1){
            for(int i = 0; i < 3; i++){
                output[i] = 0;
            } // making it fresh
            for(int i = 0; i < 3; i++){
                if (perms[i] == 0) continue;
                output[1+i*3] = rwx[0];
                output[1+i*3+1] = rwx[1];
                output[1+i*3+2] = rwx[2];
            }
        }
        if(num_p == 1){
            for(int i = 0; i < 3; i++){
                if (perms[i] == 0) continue;
                output[1+i*3] = rwx[0];
                output[1+i*3+1] = rwx[1];
                output[1+i*3+2] = rwx[2];
            }
        }
        if(num_s == 1){
            for(int i = 0; i < 3; i++){
                if (perms[i] == 0) continue;
                output[1+i*3] = (rwx[0] == 1) ? 0 : rwx[0];
                output[1+i*3+1] = (rwx[1] == 1) ? 0 : rwx[1];
                output[1+i*3+2] = (rwx[2] == 1) ? 0 : rwx[2];
            }
        }   
            return curperms;
    }
    return NULL;
}
// void iterate(int length, char ** tokens){
//     // printf("%d\n", length);
//     for (int i = 0 ; i < length; i++){
//         printf("%s\n",tokens[i]);
//         int len = strlen(tokens[i]);
//     }
// }
// #include <stdio.h>
// int ls(char * path){
//     DIR * directory = opendir(path);
//     struct dirent * curdir = readdir(directory);
//     while(curdir){

//         printf("%s\n", curdir->d_name);
//         curdir = readdir(directory);
//     }
//     return 0;
// }

// int main(){
//     int curperms[11] = {0};
//     char output[] = "----------";
//     arrayToPermStr(chmodParsing("ug=rwx",1,curperms),output);
//     printf("\n%s\n", output);
// }


