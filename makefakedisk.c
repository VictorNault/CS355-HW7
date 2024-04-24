#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "common.h"
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

#define FILE_HEADER_BYTES 16
#define DIR_ENTRY_BYTES 32
#define BLOCK_BYTES 512


FILE * global_write_fp;

void init_fattable(fat_entry local_fat_table[], fat_entry initial) {
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        local_fat_table[i] = initial;
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
    fat_entry fat_table[TOTAL_BLOCKS]; //fat_table
    //struct fattable my_fattable;
    //int initial_indicies[TOTAL_BLOCKS];
    //for (int i = 0; i < TOTAL_BLOCKS; i++) {
    //    initial_indicies[i] = UNUSED_BLOCK;
    //}
    //my_fattable.indicies = initial_indicies;
    //init_indicies(my_fattable.indicies, TOTAL_BLOCKS, UNUSED_BLOCK);
    fat_entry init_fat_entry;
    init_fat_entry.next = UNUSED_BLOCK;
    init_fattable(fat_table, init_fat_entry);

    // make initial padding for file blocks
    //int header_padding[FILE_AFTER_HEADER_BYTES];
    //for (int i = 0; i < FILE_AFTER_HEADER_BYTES; i++) {
    //    header_padding[i] = 0;
    //}

    // make root directory
    dir_header root_dir;
    strcpy(root_dir.name, "root");
    root_dir.is_directory = TRUE;
    //root_dir.FAT_entry = 0;
    root_dir.first_FAT_idx = 0;
    //root_dir.size = NAME_BYTES + (3 * sizeof(u_int8_t)) + (2 * sizeof(u_int8_t)) +  sizeof(u_int32_t);
    // also ., .., and next
    root_dir.size = FILE_HEADER_BYTES + (3 * DIR_ENTRY_BYTES);
    //root_dir.uid = 101;
    //root_dir.restrictions = READ_WRITE;
    //root_dir.protection = 202;
    //root_dir.protection[0] = TRUE;
    //root_dir.protection[1] = TRUE;
    //root_dir.protection[2] = TRUE;
    //for (int i = 3; i < 10; i++) {
    //    root_dir.protection[i] = FALSE;
    //}
    //root_dir.data_in_first_block = header_padding;
    //init_indicies(root_dir.data_in_first_block, FILE_AFTER_HEADER_BYTES, 0);
    //my_fattable.indicies[0] = -1;
    fat_entry single_block_fat_entry;
    single_block_fat_entry.next = -1;
    fat_table[0] = single_block_fat_entry;

    // make next directory
    dir_header next_dir;
    strcpy(next_dir.name, "next");
    next_dir.is_directory = TRUE;
    //next_dir.FAT_entry = 1;
    next_dir.first_FAT_idx = 1;
    //next_dir.size = NAME_BYTES + (3 * sizeof(u_int8_t)) + (2 * sizeof(u_int8_t)) +  sizeof(u_int32_t);
    next_dir.size = FILE_HEADER_BYTES  + (2 * DIR_ENTRY_BYTES);
    //next_dir.uid = 101;
    //next_dir.restrictions = READ_WRITE;
    //next_dir.protection = 202;
    //next_dir.protection[0] = TRUE;
    //next_dir.protection[1] = TRUE;
    //next_dir.protection[2] = TRUE;
    //for (int i = 3; i < 10; i++) {
    //    next_dir.protection[i] = FALSE;
    //}

    //next_dir.data_in_first_block = header_padding;
    //init_indicies(next_dir.data_in_first_block, FILE_AFTER_HEADER_BYTES, 0);
    //my_fattable.indicies[1] = -1;
    fat_table[1] = single_block_fat_entry;

    // . and .. for root
    //root_dir.data_in_first_block[0] = 0; // root_dir is always at block 0
    //root_dir.size = root_dir.size + sizeof(int);
    //root_dir.data_in_first_block[4] = -1;
    //root_dir.size = root_dir.size + sizeof(int);
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
    //root_dir.data_in_first_block[0 * DIR_ENTRY_BYTES] = root_sdot;
    root_dir.data_in_first_block[0] = root_sdot;

    struct dir_entry root_ddot;
    strcpy(root_ddot.name, "INVALID!");
    root_ddot.first_FAT_idx = UNUSED_BLOCK;
    root_ddot.size = -1;
    root_ddot.uid = 101;
    for (int i = 0; i < 10; i++) {
        root_ddot.protection[i] = FALSE;
    }
    root_dir.data_in_first_block[1] =   root_ddot;

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
    //next_dir.data_in_first_block[0] = 1;
    //next_dir.size = next_dir.size + sizeof(int);
    //next_dir.data_in_first_block[4] = 0;
    //next_dir.size = next_dir.size + sizeof(int);

    // write fat table and directories
    //fwrite(&my_fattable, FATTABLE_BYTES, 1, global_write_fp);
    fwrite(&fat_table, FATTABLE_BYTES, 1, global_write_fp);
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
