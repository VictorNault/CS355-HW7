//#include <stdlib.h>
//#include <stdio.h>

//#include "disk.h"
//#include "common.h"

/*
#define NAME_BYTES 8
#define TOTAL_BYTES 1048576
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
#define FIXED_FREEBLOCK 2
#define UNUSED_BLOCK -2
#define FAKEDISK_NAME "fake_disk"
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

#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4


#define NUM_OPEN_FILES 50
#define BLOCK_SIZE 512
#define NUM_FAT_ENTRIES 1024

//errors
#define FILE_NOT_FOUND 2

//disk
#define NAME_BYTES 9
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
#define FIXED_FREEBLOCK 1
#define UNUSED_BLOCK -2
#define FRESHDISK_NAME "fresh_disk"
#define TRUE 1
#define FALSE 0
#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4
#define FREE_DATABLOCK_EXTRA_BYTES 480
#define PROT_BYTES 12

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
    u_int8_t protection[PROT_BYTES]; //16 protection bytes
}dir_entry;

//file entry 
typedef struct file_header { //16 bytes total, 496 bytes buffer
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    char data_in_first_block[FILE_AFTER_HEADER_BYTES];
}file_header;

typedef struct dir_header { //16 bytes total, 496 bytes buffer
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t first_FAT_idx; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    dir_entry data_in_first_block[15];
    char padding[16];
}dir_header;

typedef struct free_datablock {
    int next;
    char extra[FREE_DATABLOCK_EXTRA_BYTES];
}free_datablock;

#define FILE_HEADER_BYTES 16
#define DIR_ENTRY_BYTES 32
#define BLOCK_BYTES 512
#define TOTAL_DATA_BYTES (1048576 - 512 - 8192)
#define TOTAL_DATABLOCKS ((1048576 - 512 - 8192) / 512)


FILE * global_read_fp;

//#define FILE_HEADER_BYTES 16
//#define DIR_ENTRY_BYTES 32
//#define BLOCK_BYTES 512
//#define SUPERBLOCK_BYTES 512
//#define FRESHDISK_NAME "fresh_disk"
//#define FATTABLE_BYTES 8192 // 2048 * 4




/*
struct superblock {
    int size; // size of blocks in bytes 
    int table_offset; // offset of FAT table region in blocks 
    int data_offset; // data region offset in blocks 
    //int swap_offset; // swap region offset in blocks 
    int free_block; // head of free block list, index, if disk is full, -1 
    //int data_offset;
    int fat_offset;
    char padding[SUPERBLOCK_PADDING];
};

struct fattable {
    int indicies[TOTAL_BLOCKS];
};

//file entry 
typedef struct file_entry { 
    char name[NAME_BYTES];
    u_int8_t is_directory; //1 = directory, 0 = normal file
    u_int16_t FAT_entry; //first FAT entry, 2 bytes
    u_int32_t size; //legnth of file in bytes, 4 bytes
    u_int8_t uid; //owner's user ID
    //u_int8_t restrictions; //read, write, read/write, append
    u_int8_t protection[PROT_BYTES]; //9 protection bits 
    char data_in_first_block[FILE_AFTER_HEADER_BYTES];
}file_entry;

struct free_datablock {
        int next;
        char extra[FREE_DATABLOCK_EXTRA_BYTES];
    };
*/

int main() {
    global_read_fp = fopen(FRESHDISK_NAME, "rb");

    // read superblock
    struct superblock my_superblock;
    fread(&my_superblock, SUPERBLOCK_BYTES, 1, global_read_fp);

    // read fattable
    //struct fattable my_fattable;
    fat_entry fat_table[TOTAL_BLOCKS];
    fread(&fat_table, FATTABLE_BYTES, 1, global_read_fp);

    // read directories
    dir_header root_dir;
    fread(&root_dir, BLOCK_BYTES, 1, global_read_fp);

    //dir_header next_dir;
    //fread(&next_dir, BLOCK_BYTES, 1, global_read_fp);

    // read first two free blocks
    struct free_datablock freeblock1;
    fread(&freeblock1, BLOCK_BYTES, 1, global_read_fp);

    struct free_datablock freeblock2;
    fread(&freeblock2, BLOCK_BYTES, 1, global_read_fp);

    int dummy;
    dummy = 12;
    dummy = 86;

    fclose(global_read_fp);

}
