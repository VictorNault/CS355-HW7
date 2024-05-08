#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include "disk.h"
//#include "common.h"

/*
#define NAME_BYTES 8
#define TOTAL_BYTES 1048576
#define TOTAL_DATA_BYTES (1048576 - 512 - 8192)
#define TOTAL_DATA_BLOCKS ((1048576 - 512 - 8192) / 512)
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define BLOCK_BYTES 512
#define SUPERBLOCK_PADDING 492
#define FILE_AFTER_HEADER_BYTES 512 - 32
#define TABLE_OFFSET 1
#define TABLE_BLOCKS 16
#define FIXED_FREEBLOCK 1
#define UNUSED_BLOCK -2
#define FAKEDISK_NAME "fresh_disk"
#define TRUE 1
#define FALSE 0
#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4
#define FREE_DATABLOCK_EXTRA_BYTES 508
#define PROT_BYTES 16
*/

// common.h
#define BLOCK_SIZE 512

//disk
#define NAME_BYTES 9
#define TOTAL_BYTES 1048576
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define SUPERBLOCK_PADDING 492
#define FILE_AFTER_HEADER_BYTES 480
#define TABLE_OFFSET 1
#define TABLE_BLOCKS 16
#define FIXED_FREEBLOCK 1
#define UNUSED_BLOCK -2
#define FAKEDISK_NAME "DISK"
#define TRUE 1
#define FALSE 0
#define FREE_DATABLOCK_EXTRA_BYTES 480
#define PROT_BYTES 11

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
//#include "disk.h"
//#include "List.h"
//#include "node.h"
//#include "file_system.h"
#include <assert.h>

// disk.h

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

//inside the directory data block, 32 bytes
typedef struct dir_entry{
    char name[NAME_BYTES];
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    u_int8_t uid; //owner's user ID
    u_int8_t protection[PROT_BYTES]; //11 protection bytes (2 padding)
    u_int8_t is_directory; 
}dir_entry;

//file entry 
typedef struct file_header { //16 bytes total, 496 bytes buffer
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char padding[16];
    char data_in_first_block[FILE_AFTER_HEADER_BYTES];
}file_header;

typedef struct dir_header { //16 bytes total, 496 bytes buffer
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char padding[16];
    dir_entry data_in_first_block[15];
}dir_header;

typedef struct free_datablock {
    int next;
    char extra[FREE_DATABLOCK_EXTRA_BYTES];
}free_datablock;

#define FILE_HEADER_BYTES 32
#define DIR_ENTRY_BYTES 32
#define BLOCK_BYTES 512
#define TOTAL_DATA_BYTES (1048576 - 512 - 8192)
#define TOTAL_DATABLOCKS ((1048576 - 512 - 8192) / 512)
#define ENTRIES_IN_FRESH_ROOT 2

FILE * global_write_fp;

void init_fattable(fat_entry local_fat_table[], fat_entry initial) {
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        local_fat_table[i] = initial;
    }
}


FILE * makenewdisk() {
    global_write_fp = fopen(FAKEDISK_NAME, "wb");

    // write superblock
    struct superblock my_superblock;
    my_superblock.size = BLOCK_BYTES;
    my_superblock.table_offset = TABLE_OFFSET;
    my_superblock.data_offset = TABLE_OFFSET + TABLE_BLOCKS;
    my_superblock.free_block = FIXED_FREEBLOCK;
    my_superblock.fat_offset = TABLE_OFFSET;
    // padding can just be junk data
    fwrite(&my_superblock, SUPERBLOCK_BYTES, 1, global_write_fp);

    // make fattable
    fat_entry fat_table[TOTAL_BLOCKS]; //fat_table
    fat_entry init_fat_entry;
    init_fat_entry.next = UNUSED_BLOCK;
    init_fattable(fat_table, init_fat_entry);

    // make root directory
    dir_header root_dir;
    strcpy(root_dir.name, "root");
    root_dir.is_directory = TRUE;
    root_dir.first_FAT_idx = 0;
    // also ., .., and next
    root_dir.size = FILE_HEADER_BYTES + (ENTRIES_IN_FRESH_ROOT * DIR_ENTRY_BYTES);
    fat_entry single_block_fat_entry;
    single_block_fat_entry.next = -1;
    fat_table[0] = single_block_fat_entry;

    /*
    // make next directory
    dir_header next_dir;
    strcpy(next_dir.name, "next");
    next_dir.is_directory = TRUE;
    next_dir.first_FAT_idx = 1;
    next_dir.size = FILE_HEADER_BYTES  + (2 * DIR_ENTRY_BYTES);

    fat_table[1] = single_block_fat_entry;
    */

    // . and .. for root
    struct dir_entry root_sdot;
    strcpy(root_sdot.name, root_dir.name);
    root_sdot.first_FAT_idx = root_dir.first_FAT_idx;
    root_sdot.size = root_dir.size;
    root_sdot.uid = 101;
    root_sdot.protection[0] = TRUE;
    root_sdot.protection[1] = TRUE;
    root_sdot.protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        root_sdot.protection[i] = FALSE;
    }
    root_sdot.is_directory = TRUE;
    root_dir.data_in_first_block[0] = root_sdot;

    struct dir_entry root_ddot;
    strcpy(root_ddot.name, "INVALID!");
    root_ddot.first_FAT_idx = UNUSED_BLOCK;
    root_ddot.size = -1;
    root_ddot.uid = 101;
    for (int i = 0; i < 10; i++) {
        root_ddot.protection[i] = FALSE;
    }
    root_ddot.is_directory = FALSE;
    root_dir.data_in_first_block[1] =   root_ddot;

    /*
    struct dir_entry next_dir_entry;
    strcpy(next_dir_entry.name, next_dir.name);
    next_dir_entry.first_FAT_idx = next_dir.first_FAT_idx;
    next_dir_entry.size = next_dir.size;
    next_dir_entry.uid = 101;
    next_dir_entry.protection[0] = TRUE;
    next_dir_entry.protection[1] = TRUE;
    next_dir_entry.protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        next_dir_entry.protection[i] = FALSE;
    }
    root_dir.data_in_first_block[2] =   next_dir_entry;

    // . and .. for next directory
    next_dir.data_in_first_block[0] =   next_dir_entry;
    next_dir.data_in_first_block[1] =   root_sdot;
    */
    
    // write fat table and directories
    fwrite(&fat_table, FATTABLE_BYTES, 1, global_write_fp);
    fwrite(&root_dir, BLOCK_BYTES, 1, global_write_fp);
    //fwrite(&next_dir, BLOCK_BYTES, 1, global_write_fp);
    
    // redoing making freeblocks because it's a mess
    for (int i = FIXED_FREEBLOCK; i < TOTAL_DATABLOCKS; i++) {
        struct free_datablock next_free_db;
        next_free_db.next = i + 1;
        if (next_free_db.next >= (TOTAL_DATABLOCKS)) {
            next_free_db.next = -1;
        }
        fwrite(&next_free_db, BLOCK_BYTES, 1, global_write_fp);
    }
    
    /*
    int total_freeblocks = (1 + TABLE_BLOCKS + FIXED_FREEBLOCK);
    for (int i = FIXED_FREEBLOCK; i < (TOTAL_BLOCKS - total_freeblocks); i++) {
        struct free_datablock next_free_db;
        next_free_db.next = i + 1;
        if (next_free_db.next > (TOTAL_BLOCKS - total_freeblocks)) {
            next_free_db.next = -1;
        }
        fwrite(&next_free_db, BLOCK_BYTES, 1, global_write_fp);
    }
    */
    //printf("%ld\n", sizeof(dir_entry));
    return global_write_fp;
}