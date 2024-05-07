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
    char padding[SUPERBLOCK_PADDING];
}superblock;



typedef struct free_datablock {
    int next;
    char extra[FREE_DATABLOCK_EXTRA_BYTES];
}free_datablock;




#endif