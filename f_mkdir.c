#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "disk.h"
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

#define SUCCESS 1
#define FAIL 0
#define TP_FAIL -1
#define BLOCK_BYTES 512
#define DATA_OFFSET 17
#define FILE_HEADER_BYTES 32
#define ROOT_BLOCK_ADDR 0
#define NONE_FREE -1
#define FAT_LIST_END -1
#define DIR_DO_NOT_TRACE_BYTES 16
#define PROT_BYTES 16

struct superblock global_superblock;
struct fattable global_fat_table;
*/

#define SUCCESS 1
#define FAIL 0
#define TP_FAIL -1
#define BLOCK_BYTES 512
#define DATA_OFFSET 17
//#define FILE_HEADER_BYTES 32
#define ROOT_BLOCK_ADDR 0
#define NONE_FREE -1
#define FAT_LIST_END -1
#define DIR_DO_NOT_TRACE_BYTES 16
#define PROT_BYTES 16

#define FILE_HEADER_BYTES 16
#define DIR_ENTRY_BYTES 32
#define BLOCK_BYTES 512

struct superblock global_superblock;

FILE * global_rw_fp;
struct superblock global_superblock;
fat_entry global_fattable[TOTAL_BLOCKS]; //fat_table
//struct fattable global_fattable;

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



// returns block address of path
int trace_path(const char * pathname, int n) {
    char pathname_copy[n];
    strcpy(pathname_copy, pathname);
    if (strcmp(pathname_copy, "/") == 0) {
        return ROOT_BLOCK_ADDR;
    }
    char * last_file_name;
    dir_header last_file;
    dir_header root_dir;
    // Root is "/" in unix
    // This tokenizes to empty string ""
    // It also means that the root directory would need
    // to be named empty string
    // to do this elegantly
    // So we don't do this elegantly
    // And just assume the first token is root
    //int root_done = FALSE;
    fseek(global_rw_fp, BLOCK_BYTES * DATA_OFFSET, SEEK_SET);
    fread(&root_dir, BLOCK_BYTES, 1, global_rw_fp);
    int non_header_bytes_in_root = root_dir.size - FILE_HEADER_BYTES;
    int files_in_root = non_header_bytes_in_root / DIR_ENTRY_BYTES;

    int files_in_dir = files_in_root;
    dir_header curr_dir = root_dir;
    //int non_header_bytes_in_dir = non_header_bytes_in_root;
    int files_count;
    dir_entry entry_to_check;
    int block_addr;
    int file_not_found_in_dir;
    // don't check data if is actually regular file, so can use dir_header type
    dir_header file_to_check;
    //strtok(pathname_copy, "/");
    char * next_file_name = strtok(pathname_copy, "/");
    while (TRUE) {
        if (curr_dir.is_directory == TRUE) {
            // Ignore . and ..
            files_count = 2;
        }
        else {
            files_count = 0;
        }
        file_not_found_in_dir = TRUE;
        while (files_count < files_in_dir) {
            //block_addr = (int) curr_dir.data_in_first_block[files_count * sizeof(int)];
            entry_to_check = curr_dir.data_in_first_block[files_count];
            block_addr = entry_to_check.first_FAT_idx;
            fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + block_addr), SEEK_SET);
            fread(&entry_to_check, BLOCK_BYTES, 1, global_rw_fp);
            if (strcmp(entry_to_check.name, next_file_name) == 0) {
                next_file_name = strtok(NULL, "/");
                if (next_file_name == NULL) {
                    return block_addr;
                }
                else {
                    if (file_to_check.is_directory == TRUE) {
                        curr_dir = file_to_check;
                        files_in_dir = (curr_dir.size - FILE_HEADER_BYTES) / DIR_ENTRY_BYTES;
                        file_not_found_in_dir = FALSE;
                        break;
                    }
                    else {
                        return TP_FAIL;
                    }
                }
            }
            files_count++;
        }
        if (file_not_found_in_dir == TRUE) {
            return TP_FAIL;
        }
    }
}

int f_mkdir(const char *pathname, char * new_name, char *mode) {
    // need to update for situation where only one free block (head of list)
    if (global_superblock.free_block == NONE_FREE) {
        return NONE_FREE;
    }
    if ((strlen(pathname) + 1) > NAME_BYTES) {
        return NONE_FREE;
    }
    // +1 for null char
    int dir_block = trace_path(pathname, strlen(pathname) + 1);
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + dir_block), SEEK_SET);
    dir_header parent_dir;
    //fclose(global_rw_fp);
    //global_rw_fp = fopen(FAKEDISK_NAME, "rb+");
    fread(&parent_dir, BLOCK_BYTES, 1, global_rw_fp);

    // Get second free block,
    // Remove from free list,
    // Then fix free list
    struct free_datablock head_of_free_list;
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + global_superblock.free_block), SEEK_SET);
    fread(&head_of_free_list, BLOCK_BYTES, 1, global_rw_fp);

    struct free_datablock second_free_block;
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + head_of_free_list.next), SEEK_SET);
    fread(&second_free_block, BLOCK_BYTES, 1, global_rw_fp);

    // update and write free list
    int new_dir_block = head_of_free_list.next;
    struct free_datablock updated_head_of_free_list;
    head_of_free_list.next = second_free_block.next;
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + global_superblock.free_block), SEEK_SET);
    fwrite(&head_of_free_list, BLOCK_BYTES, 1, global_rw_fp);

    // update and write fat table
    fat_entry new_fat_entry;
    new_fat_entry.next = FAT_LIST_END;
    global_fattable[new_dir_block] = new_fat_entry;
    fseek(global_rw_fp, SUPERBLOCK_BYTES, SEEK_SET);
    fwrite(&global_fattable, FATTABLE_BYTES, 1, global_rw_fp);

    // add new block to parent dir (then write)
    //int non_header_bytes_in_parent = parent_dir.size - FILE_HEADER_BYTES;
    //parent_dir.data_in_first_block[non_header_bytes_in_parent] = new_dir_block;
    //fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + dir_block), SEEK_SET);
    //fwrite(&parent_dir, BLOCK_BYTES, 1, global_rw_fp);

    // make dir_entry for new dir
    struct dir_entry new_dir_entry;
    strcpy(new_dir_entry.name, new_name);
    new_dir_entry.first_FAT_idx = new_dir_block;
    new_dir_entry.size = FILE_HEADER_BYTES + (2 * DIR_ENTRY_BYTES);
    new_dir_entry.uid = 101;
    new_dir_entry.protection[0] = TRUE;
    new_dir_entry.protection[1] = TRUE;
    new_dir_entry.protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        new_dir_entry.protection[i] = FALSE;
    }

    // add new block to parent dir (then write)
    int non_header_bytes_in_parent = parent_dir.size - FILE_HEADER_BYTES;
    int files_in_parent = non_header_bytes_in_parent / DIR_ENTRY_BYTES;
    parent_dir.data_in_first_block[files_in_parent] = new_dir_entry;
    parent_dir.size = parent_dir.size + DIR_ENTRY_BYTES;
    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + dir_block), SEEK_SET);
    fwrite(&parent_dir, BLOCK_BYTES, 1, global_rw_fp);


    // finally make and write new dir itself
    dir_header new_dir;
    strcpy(new_dir.name, new_name);
    new_dir.is_directory = TRUE;
    new_dir.first_FAT_idx = new_dir_block;
    new_dir.size = new_dir_entry.size;
    new_dir.data_in_first_block[0] = new_dir_entry;
    new_dir.data_in_first_block[1] = parent_dir.data_in_first_block[0];

    fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + new_dir_block), SEEK_SET);
    fwrite(&new_dir, BLOCK_BYTES, 1, global_rw_fp);
    return 0;
}


// testing
int main() {
    //global_rw_fp = fopen(FAKEDISK_NAME, "rb+");
    global_rw_fp = fopen(FAKEDISK_NAME, "rb+");

    // read superblock
    fread(&global_superblock, SUPERBLOCK_BYTES, 1, global_rw_fp);

    // read fattable
    fread(&global_fattable, FATTABLE_BYTES, 1, global_rw_fp);

    char my_str1[2] = "/";
    int my_result = trace_path(my_str1, 2);
    printf("%d\n", my_result);

    char my_str2[7] = "/next/";
    my_result = trace_path(my_str2, 7);
    printf("%d\n", my_result);

    char my_str3[6] = "/next";
    my_result = trace_path(my_str3, 6);
    printf("%d\n", my_result);

    char my_str4[10] = "/not_next";
    my_result = trace_path(my_str4, 10);
    printf("%d\n", my_result);

    char my_str5[15] = "/next/not_next";
    my_result = trace_path(my_str5, 15);
    printf("%d\n", my_result);

    //printf("%d\n", trace_path("/next"));
    //printf("%d\n", trace_path("/not_next"));
    //printf("%d\n", trace_path("/next/not_next"));


    f_mkdir("/", "mde_dir", "mode");

    fclose(global_rw_fp);
    return 0;
}
