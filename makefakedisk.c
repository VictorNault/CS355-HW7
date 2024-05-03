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
#define FILE_AFTER_HEADER_BYTES 480
#define TABLE_OFFSET 1
#define TABLE_BLOCKS 16
#define FIXED_FREEBLOCK 1
#define UNUSED_BLOCK -2
#define FAKEDISK_NAME "fake_disk"
#define TRUE 1
#define FALSE 0
#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4
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
#define FAT_LIST_END -1
#define ROOT_INDEX 0
#define DATA_OFFSET 17
#define ENTRIES_IN_FRESH_ROOT 2
#define MB_FILENAME "beemovie"

FILE * global_rw_fp;

void init_fattable(fat_entry local_fat_table[], fat_entry initial) {
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        local_fat_table[i] = initial;
    }
}


int main() {
    global_rw_fp = fopen(FAKEDISK_NAME, "wb+");

    // write superblock
    struct superblock my_superblock;
    my_superblock.size = BLOCK_BYTES;
    my_superblock.table_offset = TABLE_OFFSET;
    my_superblock.data_offset = TABLE_OFFSET + TABLE_BLOCKS;
    my_superblock.free_block = FIXED_FREEBLOCK;
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    my_superblock.fat_offset = TABLE_OFFSET;
    // padding can just be junk data
    fwrite(&my_superblock, SUPERBLOCK_BYTES, 1, global_rw_fp);

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
    fwrite(&fat_table, FATTABLE_BYTES, 1, global_rw_fp);
    fwrite(&root_dir, BLOCK_BYTES, 1, global_rw_fp);
    //fwrite(&next_dir, BLOCK_BYTES, 1, global_rw_fp);
    
    // redoing making freeblocks because it's a mess
    for (int i = FIXED_FREEBLOCK; i < TOTAL_DATABLOCKS; i++) {
        struct free_datablock next_free_db;
        next_free_db.next = i + 1;
        //printf("next: %d\n", next_free_db.next);
        if (next_free_db.next >= (TOTAL_DATABLOCKS)) {
            next_free_db.next = -1;
        }
        fwrite(&next_free_db, BLOCK_BYTES, 1, global_rw_fp);
    }

    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    
    /*
    int total_freeblocks = (1 + TABLE_BLOCKS + FIXED_FREEBLOCK);
    for (int i = FIXED_FREEBLOCK; i < (TOTAL_BLOCKS - total_freeblocks); i++) {
        struct free_datablock next_free_db;
        next_free_db.next = i + 1;
        if (next_free_db.next > (TOTAL_BLOCKS - total_freeblocks)) {
            next_free_db.next = -1;
        }
        fwrite(&next_free_db, BLOCK_BYTES, 1, global_rw_fp);
    }
    */
    //printf("%ld\n", sizeof(dir_entry));

    // Get second free block,
    // Remove from free list,
    // Then fix free list
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    struct free_datablock head_of_free_list;
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + my_superblock.free_block), SEEK_SET);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fread(&head_of_free_list, BLOCK_BYTES, 1, global_rw_fp);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    //printf("HOFL.next: %d\n", head_of_free_list.next);

    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);

    struct free_datablock second_free_block;
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + head_of_free_list.next), SEEK_SET);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fread(&second_free_block, BLOCK_BYTES, 1, global_rw_fp);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);

    struct free_datablock third_free_block;
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + second_free_block.next), SEEK_SET);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fread(&third_free_block, BLOCK_BYTES, 1, global_rw_fp);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);

    struct free_datablock fourth_free_block;
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + third_free_block.next), SEEK_SET);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fread(&fourth_free_block, BLOCK_BYTES, 1, global_rw_fp);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    printf("I hate C.\n");
    printf("Refreshing value of my_superblock.free_block.\n");
    my_superblock.free_block = FIXED_FREEBLOCK; // I don't exen know man

    // update and write free list
    //printf("Original: %d\n", head_of_free_list.next);
    int new_multiblock_file_index = head_of_free_list.next;
    head_of_free_list.next = second_free_block.next;
    //printf("New: %d\n", head_of_free_list.next);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + my_superblock.free_block), SEEK_SET);
    //printf("Location: %d\n", (DATA_OFFSET + my_superblock.free_block));
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fwrite(&head_of_free_list, BLOCK_BYTES, 1, global_rw_fp);
    //printf("New: %d\n", head_of_free_list.next);

    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    int second_mb_file_index = third_free_block.next;
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    third_free_block.next = fourth_free_block.next;
    //printf("next: %d\n", fourth_free_block.next);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + head_of_free_list.next), SEEK_SET);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);
    fwrite(&third_free_block, BLOCK_BYTES, 1, global_rw_fp);
    printf("my_superblock.free_block should be 1: %d\n", my_superblock.free_block);

    // update and write fat table
    fat_entry new_fat_entry1;
    new_fat_entry1.next = second_mb_file_index;
    fat_table[new_multiblock_file_index] = new_fat_entry1;
    fat_entry new_fat_entry2;
    new_fat_entry2.next = FAT_LIST_END;
    fat_table[second_mb_file_index] = new_fat_entry2;
    fseek(global_rw_fp, SUPERBLOCK_BYTES, SEEK_SET);
    fwrite(&fat_table, FATTABLE_BYTES, 1, global_rw_fp);

    // make dir_entry for new dir
    struct dir_entry new_dir_entry;
    strcpy(new_dir_entry.name, MB_FILENAME);
    new_dir_entry.first_FAT_idx = new_multiblock_file_index;
    new_dir_entry.size = BLOCK_BYTES + 369;
    new_dir_entry.uid = 101;
    new_dir_entry.protection[0] = TRUE;
    new_dir_entry.protection[1] = TRUE;
    new_dir_entry.protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        new_dir_entry.protection[i] = FALSE;
    }
    new_dir_entry.is_directory = FALSE;

    // add new block to parent dir (then write)
    root_dir.data_in_first_block[2] = new_dir_entry;
    root_dir.size = root_dir.size + DIR_ENTRY_BYTES;
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + ROOT_INDEX), SEEK_SET);
    fwrite(&root_dir, BLOCK_BYTES, 1, global_rw_fp);
    
    // finally make and write new file itself
    file_header mb_file;
    strcpy(mb_file.name, MB_FILENAME);
    mb_file.is_directory = FALSE;
    mb_file.first_FAT_idx = new_multiblock_file_index;
    mb_file.size = BLOCK_BYTES + 369;
    strcpy(mb_file.data_in_first_block, "According to all known laws of aviation, there is no way a bee should be able to fly. Its wings are too small to get its fat little body off the ground. The bee, of course, flies anyway because bees don't care what humans think is impossible. Yellow, black. Yellow, black. Yellow, black. Yellow, black. Ooh, black and yellow! Let's shake it up a little. Barry! Breakfast is ready! Coming! Hang on a second. Hello? - Barry? - Adam? - Can you believe this is happening? - I can't. ");

    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + new_multiblock_file_index), SEEK_SET);
    fwrite(&mb_file, BLOCK_BYTES, 1, global_rw_fp);

    char mb_file_second_block_data[BLOCK_BYTES] = "I'll pick you up. Looking sharp. Use the stairs. Your father paid good money for those. Sorry. I'm excited. Here's the graduate. We're very proud of you, son. A perfect report card, all B's. Very proud. Ma! I got a thing going here. - You got lint on your fuzz. - Ow! That's me! - Wave to us! We'll be in row 118,000. - Bye! Barry, I told you, stop flying in the house!";

    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + second_mb_file_index), SEEK_SET);
    fwrite(mb_file_second_block_data, BLOCK_BYTES, 1, global_rw_fp);

    fclose(global_rw_fp);
    return 0;
}
