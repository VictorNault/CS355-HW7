#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "common.h"

//file handle
typedef struct file{ 
    char *name; //name of file
    size_t cur_rindex; //current read index in bytes
    size_t cur_windex; //current write index in bytes
    char * cur_rchar; //current location of character for reading
    char * cur_wchar; //current location of character for writing
    size_t size; //size of file in bytes
    size_t FAT_entry; //first FAT entry
}file;

// //directory handle
// typedef struct dir{ 
//     char *name;
//     List *files; //linked list of files in the directory
//     file_entry *current_file; //pointer to current file for readdir
//     size_t FAT_entry; //first FAT entry
// }dir;

file *f_open(const char *pathname, const int mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, file *stream);
size_t f_write(const void *ptr, size_t size, size_t nmemb, file *stream);
int f_close(file *stream);
int f_seek(file *stream, long offset, int position);
void f_rewind(file *stream);
int f_stat(file *stream, file_entry *stat_buffer);
int f_remove(file *stream);
file_entry *f_opendir(const char *name);
file_entry *f_readdir(file_entry *directory);
int f_closedir(file_entry *stream);
int f_mkdir(const char *pathname, char *mode);
int f_rmdir(const char *pathname);
extern int f_error;

#endif 