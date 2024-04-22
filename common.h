#ifndef COMMON_H
#define COMMON_H


#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4


#define NUM_OPEN_FILES 50
#define NAME_BYTES 8
#define BLOCK_SIZE 512
#define NUM_FAT_ENTRIES 1024

//errors
#define FILE_NOT_FOUND 2

//disk
#define NAME_BYTES 8
#define TOTAL_BYTES 1048576
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define SUPERBLOCK_PADDING 492
#define FILE_AFTER_HEADER_BYTES 496
#define TABLE_OFFSET 1
#define TABLE_BLOCKS 16
#define FIXED_FREEBLOCK 2
#define UNUSED_BLOCK -2
#define FAKEDISK_NAME "fake_disk"
#define TRUE 1
#define FALSE 0
#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4
#define FREE_DATABLOCK_EXTRA_BYTES 480
#define PROT_BYTES 16

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "disk.h"
#include "List.h"
#include "node.h"
#include "file_system.h"
#include <assert.h>


extern int is_initialized;
extern file_handle *open_files[];
extern FILE *disk;
extern int table_offset; /* offset of FAT table region in blocks */
extern int data_offset; /* data region offset in blocks */
extern int free_block; /* head of free block list, index, if disk is full, -1 */
extern int fat_offset;
extern fat_entry fat_table[TOTAL_BLOCKS]; //fat_table
#endif