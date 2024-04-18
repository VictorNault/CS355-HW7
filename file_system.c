#include "common.h"
#include "file_system.h"

file_entry open_files[NUM_OPEN_FILES]; //array for open files
FILE *disk;
int data_os = 0;
int fat_os = 0;
fat_entry fat_table[NUM_FAT_ENTRIES];
int f_error = EXIT_SUCCESS;

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

unsigned long find_offset(int block){
    //finds the byte offset from 0 for the data block
    return data_os * BLOCK_SIZE + BLOCK_SIZE + block * BLOCK_SIZE;
}

fat_entry *find_next_fat(fat_entry *current){
    int index = current->next;
    if(index == -1){
        return NULL;
    }
    return &fat_table[index];
}

file_entry *find_file_from_directory(file_entry *dir, fat_entry *cur_fat, char *name){
    //returns the file entry we are looking for in a directory
    file_entry *cur_file = dir;
    int counter = 0;
    do{
        while(counter * sizeof(file_entry) < BLOCK_SIZE){
            counter ++;
            cur_file = cur_file + 1;
            if(strcmp(cur_file->name, name) == 0){
                return cur_file;
            }
        }
        cur_file = (file_entry*)find_offset(cur_fat->next);
        cur_fat = find_next_fat(cur_fat);
        counter = 0;
    }while(cur_fat);

    return NULL;
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
    //opens a directory file for reading and returns a file_entry
    fat_entry *fat_e = malloc(sizeof(fat_entry));
    file_entry *file_e = malloc(sizeof(file_entry));
    long cur_block = 0;

    if(!disk){
        disk = fopen("disk_image","w+");
    }
    //***needs to read superblock***

    //tokenizing the pathname
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    
    //seeking the root directory fat entry
    fseek(disk,find_offset(fat_os),SEEK_SET); 
    fread(fat_e,sizeof(*fat_e),1,disk);

    //seeking the data block for root
    fseek(disk,find_offset(cur_block),SEEK_SET);
    fread(file_e,sizeof(*file_e),1,disk);

    //finding file from directory repeatedly
    for(int i = 0; i < token_length; i++){
        file_e = find_file_from_directory(file_e,fat_e,tokens[i]);
        
        //error checking
        if(!file_e && i < token_length - 1){
            printf("Directory does not exist, exiting f_open\n");
            f_error = FILE_NOT_FOUND;
            return NULL;
            
        }
        else if(!file_e && i == token_length - 1 && mode == READ_ONLY){
            printf("File does not exist in read mode, exiting f_open\n");
            f_error = FILE_NOT_FOUND;
            return NULL;
        }
        // else if(!file_e && i == token_length - 1){
        //     printf("File does not exist, making file...\n");
        //     make_file();
        // }

        //update fat_entry
        fat_e = &fat_table[file_e->FAT_entry];
    }
    
    //making a new file struct
    file *to_return = malloc(sizeof(file));
    to_return->cur_rindex = 0; 
    to_return->cur_windex = 0;
    to_return->name = tokens[token_length-1];
    to_return->size = file_e->size;

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
    //close a file handle, cleans up memory in the open files list
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

file_entry *f_opendir(const char *pathname){
    //opens a directory file for reading and returns a file_entry
    fat_entry *fat_e = malloc(sizeof(fat_entry));
    file_entry *file_e = malloc(sizeof(file_entry));
    long cur_block = 0;

    if(!disk){
        disk = fopen("disk_image","w+");
    }
    //***needs to read superblock***

    //tokenizing the pathname
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    
    //seeking the root directory fat entry
    fseek(disk,find_offset(fat_os),SEEK_SET); 
    fread(fat_e,sizeof(*fat_e),1,disk);

    //seeking the data block for root
    fseek(disk,find_offset(cur_block),SEEK_SET);
    fread(file_e,sizeof(*file_e),1,disk);

    //finding file from directory repeatedly
    for(int i = 0; i < token_length; i++){
        file_e = find_file_from_directory(file_e,fat_e,tokens[i]);
        //update fat_entry
        if(!file_e){
            printf("Directory not found, exiting f_opendir\n");
            f_error = FILE_NOT_FOUND;
            return NULL;
        }
        fat_e = &fat_table[file_e->FAT_entry];
    }
    return file_e;

    //cleaning up
    for(int i = 0; i < token_length; i++){
        free(tokens[i]);
    }
    free(tokens);
    return NULL;
}

file_entry *f_readdir(file_entry *directory){
    //returns the next file_entry in the directory, updates the pointer to the current file
}

int f_closedir(file_entry *stream){
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

}