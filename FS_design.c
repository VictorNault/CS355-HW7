//Design document for file system library functions and data structures
#include <stdio.h>
#include <stdlib.h>
LIST *open_files = NULL; //linked list for open files

//file handle
typedef struct file{ 
    char *name;
    char *read_current;
    char *read_end;
    char *write_current;
    char *write_end;
}file;

//directory handle
typedef struct dir{ 
    char *name;
    LIST *files;
}dir;

//directory entry
typedef struct dir_entry { 
    char *name;
    size_t FAT_entry; //FAT entry
    unsigned short length;
    unsigned char type;
}dir_entry;

//file status
typedef struct stat { 
    char *name; //name of file
    char *type; //file type
    size_t size; //file size in bytes
    size_t num_blocks; //file size in blocks
    char *protection; //access permissions
    size_t uid; //owner's user ID
}stat;


file *f_open(const char *pathname, const char *mode){
    //open a file with specified access mode
    //read, write, read/write, append
    //if file does not exist, create the file in the specified directory
    //returns file pointer if successful
}

size_t f_read(void *ptr, size_t size, size_t nmemb, file *stream){

}

size_t f_write(const void *ptr, size_t size, size_t nmemb, file *stream){

}

int f_close(file *stream){

}

int f_seek(file *stream, long offset, int position){

}

void f_rewind(file *stream){
    
}

int f_stat(file *stream, stat *stat_buffer){

}

int f_remove(file *stream){

}

dir *f_opendir(const char *name){

}

dir_entry *f_readdir(dir *directory){

}

int f_closedir(dir_entry *stream){

}

int f_mkdir(const char *pathname, char *mode){

}

int f_rmdir(const char *pathname){

}


