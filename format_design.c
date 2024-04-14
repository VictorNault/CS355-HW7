#include <stdlib.h>
#include <stdio.h>

#define DISK_SIZE 1000000
#define N_BLOCKS 2
struct fat_entry{
    int next;
    int data;
};
struct superblock {
    int size; /* size of blocks in bytes */
    int table_offset; /* offset of FAT table region in blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_block; /* head of free block list, index, if disk is full, -1 */
};

struct fat_entry* r_block = NULL;
struct fat_entry** table = NULL;
int format(){
    /*
        1. establish the (fat) table with malloc(DISK_SIZE)
        2. set table[0] to the r_block, which is the root block.
        3. create a file if table[1] is NULL
        4. table[1] data = &file (file created using fopen).
        5. the header contains blocks of data from the data region, along with
        the location in the FAT table. 
        6. the last two blocks in the table are set to -1. r_block and the new file point to them in their 'next'
        array. (for bare bones)
        7. use fwrite to write the file at a specific size in a specific place.
        8. fill the rest of the data section with 0s to symbolize the fact that they are empty. 
        Tree movement for root directory: data for root directory has list of data block locations
        for each file right below it. Thus, to get all of the files you have to just go through that list.
    */
    // table = malloc(DISK_SIZE);
    // r_block = malloc(sizeof(struct fat_entry));
    // table[0] = r_block;
    // r_block->next[0] = 1;
    // r_block->next[1] = 3;
    return 0;
}

int main(){
    format();
    return 0;
}
