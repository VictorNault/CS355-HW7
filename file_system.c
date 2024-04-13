//Design document for file system library functions and data structures
#include "common.h"

List *open_files = NULL; //linked list for open files

//file handle
typedef struct file{ 
    char *name;
    size_t read_current;
    size_t read_end;
    size_t write_current;
    size_t write_end;
    size_t FAT_entry; //first FAT entry
}file;

//directory handle
typedef struct dir{ 
    char *name;
    List *files; //linked list of files in the directory
    file_entry *current_file; //pointer to current file for readdir
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
    u_int8_t restrictions; //read, write, read/write, append
    u_int16_t protection; //9 protection bits
}file_entry;

void update_protection(u_int16_t prot, const char *pathname){
    //updates the protection bits of a file specified at pathname
}

file *f_open(const char *pathname, const int mode){
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
}

int f_rmdir(const char *pathname){
    //delete a directory, removes entire contents and the contents of all subdirectorys from the filesystem
}


