#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "string_extras.h"
#include <dirent.h>
#include <sys/types.h>
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
//     ls("/home/kbritt1/Documents/cs355_Hw/CS355-HW7/FS-shell/");
// }


