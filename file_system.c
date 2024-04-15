#include "common.h"
#include "file_system.h"

List *open_files = NULL; //linked list for open files

//file entry 
typedef struct file_entry { 
    char *pathname; 
    u_int8_t type; //type of file
    u_int64_t time; //time of creation in seconds, 8 bytes
    size_t FAT_entry; //first FAT entry, 2 bytes
    u_int32_t length; //legnth of file in bytes, 4 bytes
    u_int8_t uid; //owner's user ID
    u_int8_t restrictions; //read, write, read/write, append
    u_int16_t protection; //9 protection bits 
}file_entry;

//datablock for directory, needs coordination with Hilary
typedef struct dir_datablock{

}dir_datablock;

char ** tokenize(const char * stringToSplit, int * cmdLen, char* delimiters){ 
    //counting the number of arguments passed by calling strtok twice (not the most efficient :()
    char * stringToSplitCopy = malloc( sizeof(char) * (strlen(stringToSplit)+1));
    strcpy(stringToSplitCopy, stringToSplit); 
    char * token = strtok(stringToSplitCopy,delimiters);
    *cmdLen = 0;
    while (token != NULL){
        (*cmdLen)++;
        token = strtok(NULL,delimiters);
    }

    if (*cmdLen == 0){ //nothing was passed
        return NULL;
    }

    char ** tokenList = (char **) malloc(sizeof(char *) * (*cmdLen));

    strcpy(stringToSplitCopy, stringToSplit); 
    token = strtok(stringToSplitCopy, delimiters);
    tokenList[0] = (char *) malloc(sizeof(char)* (strlen(token)+1));
    strcpy(tokenList[0],token);
    for (int i = 1; i < *cmdLen; i++){
        token = strtok(NULL, delimiters);
        tokenList[i] = (char *) malloc(sizeof(char)* (strlen(token)+1));
        strcpy(tokenList[i],token);
    }

    free(stringToSplitCopy);
    return tokenList;
}

file_entry *get_file_entry(const char *pathname){
    //returns the file entry to a specific file
}

void update_file_entry(file_entry *file_to_update){
    //updates the file entry in the corresponding FAT entry and the data block header
}

// void terminate(){
//     clear(open_files);
// }

file *f_open(const char *pathname, const int mode){
    //open a file with specified access mode
    //read, write, read/write, append
    //if file does not exist, create the file in the specified directory
    //returns file pointer if successful

    if(!open_files){ //initialize open files list
        open_files = newList();
    }

    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    




    //cleaning up
    for(int i = 0; i < token_length; i++){
        free(tokens[i]);
    }
    free(tokens);
    return NULL;
    
}

size_t f_read(void *ptr, size_t size, size_t nmemb, file *stream){
    //read the specified number of bytes from a file handle at the current position. 
    //returns the number of bytes read, or an error.
}

size_t f_write(const void *ptr, size_t size, size_t nmemb, file *stream){
    //write some bytes to a file handle at the current position. 
    //Returns the number of bytes written, or an error.
}

int f_close(file *stream){
    //close a file handle, cleans up memory in thr open files list
}

int f_seek(file *stream, long offset, int position){
    //move pointers to a specified position in a file
}

void f_rewind(file *stream){
    //move pointers to the start of the file
}

int f_stat(file *stream, file_entry *stat_buffer){
    //retrieve information about a file
    //updates the stat_buffer struct
}

int f_remove(file *stream){
    //delete a file from disk
    //returns EXIT_SUCCESS if successfully deleted or error
}

dir *f_opendir(const char *name){
    //opens a directory file for reading and returns a directory handle
}

file_entry *f_readdir(dir *directory){
    //returns the next file_entry in the directory, updates the pointer to the current file
}

int f_closedir(dir *stream){
    //close an open directory file, cleans up memory in the open files list
}

int f_mkdir(const char *pathname, char *mode){
    //creates a new directory file in the specified location
    //make sure to update values for ./ and ../
}

int f_rmdir(const char *pathname){
    //delete a directory, removes entire contents and the contents of all subdirectorys from the filesystem
}

int main(){
    f_open("/usr/bjiang/home/Desktop/cs.txt",READONLY);
}