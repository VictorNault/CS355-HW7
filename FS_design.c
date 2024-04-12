//Design document for file system library functions and data structures
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
LIST *open_files = NULL; //linked list for open files

//file handle
typedef struct file{ 
    char *name;
    char *read_current;
    char *read_end;
    char *write_current;
    char *write_end;
    size_t FAT_entry; //first FAT entry
}file;

//directory handle
typedef struct dir{ 
    char *name;
    LIST *files;
    size_t FAT_entry; //first FAT entry
}dir;

//file entry 
typedef struct file_entry { 
    char *name;
    u_int8_t type; //type of file
    u_int64_t time; //time of creation in seconds, 8 bytes
    size_t FAT_entry; //first FAT entry, 2 bytes
    u_int32_t length; //legnth of file in bytes, 4 bytes
    u_int8_t uid; //owner's user ID
}file_entry;


file *f_open(const char *pathname, const char *mode){
    //open a file with specified access mode
    //read, write, read/write, append
    //if file does not exist, create the file in the specified directory
    //returns file pointer if successful
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
    //close a file handle
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

}

dir *f_opendir(const char *name){

}

file_entry *f_readdir(dir *directory){

}

int f_closedir(file_entry *stream){

}

int f_mkdir(const char *pathname, char *mode){

}

int f_rmdir(const char *pathname){

}


