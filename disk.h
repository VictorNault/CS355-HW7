#ifndef DISK_H
#define DISK_H
#include "common.h"

typedef struct fat_entry{
    int next;
}fat_entry;

typedef struct superblock {
    int size; /* size of blocks in bytes */
    int table_offset; /* offset of FAT table region in blocks */
    int data_offset; /* data region offset in blocks */
    int free_block; /* head of free block list, index, if disk is full, -1 */
    int fat_offset;
    char padding[SUPERBLOCK_PADDING];
}superblock;

//file header 
typedef struct file_header { //32 bytes total, 480 bytes buffer
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char padding[16]; //junk
    char data_in_first_block[FILE_AFTER_HEADER_BYTES];
}file_header;

//inside the directory data block, 32 bytes
typedef struct dir_entry{
    char name[NAME_BYTES];
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    u_int8_t uid; //owner's user ID
    u_int8_t protection[PROT_BYTES]; //11 protection bytes (2 padding)
    u_int8_t is_directory; 
}dir_entry;

//directory header
typedef struct dir_header { //16 bytes total, 15 dir_entries, 16 byte padding
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char padding[16]; //junk
    dir_entry data_in_first_block[15]; //15 dir_entries
}dir_header;

typedef struct free_datablock {
    int next;
    char extra[FREE_DATABLOCK_EXTRA_BYTES];
}free_datablock;




#endif