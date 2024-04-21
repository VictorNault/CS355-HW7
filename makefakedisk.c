#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NAME_BYTES 8
#define TOTAL_BYTES 1048576
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define BLOCK_BYTES 512
#define SUPERBLOCK_PADDING 492
#define FILE_AFTER_HEADER_BYTES 512 - (8 + (3 * sizeof(u_int8_t)) + (2 * sizeof(u_int8_t)) +  sizeof(u_int32_t))
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

FILE * global_write_fp;

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

void init_indicies(int indicies[], int n, int initial) {
    for (int i = 0; i < n; i++) {
        indicies[i] = initial;
    }
}

int main() {
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
    struct fattable my_fattable;
    //int initial_indicies[TOTAL_BLOCKS];
    //for (int i = 0; i < TOTAL_BLOCKS; i++) {
    //    initial_indicies[i] = UNUSED_BLOCK;
    //}
    //my_fattable.indicies = initial_indicies;
    init_indicies(my_fattable.indicies, TOTAL_BLOCKS, UNUSED_BLOCK);

    // make initial padding for file blocks
    //int header_padding[FILE_AFTER_HEADER_BYTES];
    //for (int i = 0; i < FILE_AFTER_HEADER_BYTES; i++) {
    //    header_padding[i] = 0;
    //}

    // make root directory
    file_entry root_dir;
    strcpy(root_dir.name, "root");
    root_dir.is_directory = TRUE;
    root_dir.FAT_entry = 0;
    root_dir.size = NAME_BYTES + (3 * sizeof(u_int8_t)) + (2 * sizeof(u_int8_t)) +  sizeof(u_int32_t);
    root_dir.uid = 101;
    root_dir.restrictions = READ_WRITE;
    root_dir.protection = 202;
    //root_dir.data_in_first_block = header_padding;
    //init_indicies(root_dir.data_in_first_block, FILE_AFTER_HEADER_BYTES, 0);
    my_fattable.indicies[0] = -1;

    // make next directory
    file_entry next_dir;
    strcpy(next_dir.name, "next");
    next_dir.is_directory = TRUE;
    next_dir.FAT_entry = 1;
    next_dir.size = NAME_BYTES + (3 * sizeof(u_int8_t)) + (2 * sizeof(u_int8_t)) +  sizeof(u_int32_t);
    next_dir.uid = 101;
    next_dir.restrictions = READ_WRITE;
    next_dir.protection = 202;
    //next_dir.data_in_first_block = header_padding;
    //init_indicies(next_dir.data_in_first_block, FILE_AFTER_HEADER_BYTES, 0);
    my_fattable.indicies[1] = -1;

    // . and .. for root
    root_dir.data_in_first_block[0] = 0; // root_dir is always at block 0
    root_dir.size = root_dir.size + sizeof(int);
    root_dir.data_in_first_block[1] = -1;
    root_dir.size = root_dir.size + sizeof(int);

    // Put next directory in root directory
    root_dir.data_in_first_block[3] = next_dir.FAT_entry;
    root_dir.size = root_dir.size + sizeof(int);

    // . and .. for next directory
    next_dir.data_in_first_block[0] = 1;
    next_dir.size = next_dir.size + sizeof(int);
    next_dir.data_in_first_block[1] = 0;
    next_dir.size = next_dir.size + sizeof(int);

    // write fat table and directories
    fwrite(&my_fattable, FATTABLE_BYTES, 1, global_write_fp);
    fwrite(&root_dir, BLOCK_BYTES, 1, global_write_fp);
    fwrite(&next_dir, BLOCK_BYTES, 1, global_write_fp);

    int total_freeblocks = (1 + TABLE_BLOCKS + FIXED_FREEBLOCK);
    for (int i = FIXED_FREEBLOCK; i < (TOTAL_BLOCKS - total_freeblocks); i++) {
        struct free_datablock next_free_db;
        next_free_db.next = i + 1;
        if (next_free_db.next > (TOTAL_BLOCKS - total_freeblocks)) {
            next_free_db.next = -1;
        }
        fwrite(&next_free_db, BLOCK_BYTES, 1, global_write_fp);
    }

    fclose(global_write_fp);
    return 0;
}
