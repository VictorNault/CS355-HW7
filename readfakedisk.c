#include <stdlib.h>
#include <stdio.h>

#define NAME_BYTES 8
#define TOTAL_BYTES 1048576
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define BLOCK_BYTES 512
#define SUPERBLOCK_PADDING 492
#define FILE_AFTER_HEADER_BYTES 416
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


FILE * global_read_fp;

struct superblock {
    int size; /* size of blocks in bytes */
    int table_offset; /* offset of FAT table region in blocks */
    int data_offset; /* data region offset in blocks */
    //int swap_offset; /* swap region offset in blocks */
    int free_block; /* head of free block list, index, if disk is full, -1 */
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
    u_int8_t restrictions; //read, write, read/write, append
    u_int16_t protection; //9 protection bits 
    char data_in_first_block[FILE_AFTER_HEADER_BYTES];
}file_entry;

struct free_datablock {
        int next;
        char extra[FREE_DATABLOCK_EXTRA_BYTES];
    };

int main() {
    global_read_fp = fopen(FAKEDISK_NAME, "rb");

    // read superblock
    struct superblock my_superblock;
    fread(&my_superblock, SUPERBLOCK_BYTES, 1, global_read_fp);

    // read fattable
    struct fattable my_fattable;
    fread(&my_fattable, FATTABLE_BYTES, 1, global_read_fp);

    // read directories
    file_entry root_dir;
    fread(&root_dir, BLOCK_BYTES, 1, global_read_fp);

    file_entry next_dir;
    fread(&next_dir, BLOCK_BYTES, 1, global_read_fp);

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
