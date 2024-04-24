#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "common.h"

#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4

//file handle
typedef struct file_handle{ 
    char name[NAME_BYTES]; //name of file
    size_t cur_rindex; //current read index in bytes
    size_t cur_windex; //current write index in bytes
    char * cur_rchar; //current location of character for reading
    char * cur_wchar; //current location of character for writing
    size_t size; //size of file in bytes
    u_int16_t first_FAT_idx; //first FAT entry
}file_handle;

// //directory handle
// typedef struct dir{ 
//     char *name;
//     List *files; //linked list of files in the directory
//     file_header *current_file; //pointer to current file for readdir
//     size_t FAT_entry; //first FAT entry
// }dir;

file_handle *f_open(const char *pathname, const int mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, file_handle *stream);
size_t f_write(const void *ptr, size_t size, size_t nmemb, file_handle *stream);
int f_close(file_handle *stream);
int f_seek(file_handle *stream, long offset, int position);
void f_rewind(file_handle *stream);
int f_stat(file_handle *stream, file_header *stat_buffer);
int f_remove(file_handle *stream);
file_header *f_opendir(const char *name);
file_header *f_readdir(file_header *directory);
int f_closedir(file_header *stream);
int f_mkdir(const char *pathname, char *mode);
int f_rmdir(const char *pathname);

extern int f_error;
extern FILE *disk;


#endif 