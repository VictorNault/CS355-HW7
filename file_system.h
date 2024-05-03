#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H



#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4

//file handle
typedef struct file_handle{ 
    char name[NAME_BYTES]; //name of file
    u_int8_t is_dir; //is the file a directory
    size_t cur_rindex; //current read index in bytes
    size_t cur_windex; //current write index in bytes
    size_t size; //size of file in bytes
    u_int16_t first_FAT_idx; //first FAT entry = first block of file
}file_handle;

//file header 
typedef struct file_header { //32 bytes total, 480 bytes buffer
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char padding[16]; //junk
    char data_in_first_block[FILE_AFTER_HEADER_BYTES];
}file_header;

//directory header
typedef struct dir_header { //16 bytes total, 15 dir_entries, 16 byte padding
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char padding[16]; //junk
    dir_entry data_in_first_block[15]; //15 dir_entries
}dir_header;

void f_init();
file_handle *f_open(const char *pathname, const int mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, file_handle *stream);
size_t f_write(const void *ptr, size_t size, size_t nmemb, file_handle *stream);
int f_close(file_handle *stream);
int f_seek(file_handle *stream, long offset, int position);
void f_rewind(file_handle *stream);
int f_stat(file_handle *stream, file_header *stat_buffer);
int f_remove(file_handle *stream);
file_handle *f_opendir(const char *name);
file_handle *f_readdir(dir_header *directory);
int f_closedir(file_header *stream);
int f_mkdir(const char *pathname, char *mode);
int f_rmdir(const char *pathname);

extern int f_error;
extern FILE *disk;


#endif 