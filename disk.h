#ifndef DISK_H
#define DISK_H
#include "common.h"

typedef struct fat_entry{
    u_int16_t next;
}fat_entry;

//file entry 
typedef struct file_entry { 
    char name[NAMEBYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t FAT_entry; //first FAT entry, 2 bytes
    u_int32_t length; //legnth of file in bytes, 4 bytes
    u_int8_t uid; //owner's user ID
    u_int8_t restrictions; //read, write, read/write, append
    u_int16_t protection; //9 protection bits 
}file_entry;

#endif