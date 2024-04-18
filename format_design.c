#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define DISK_SIZE 1000000
#define N_BLOCKS 2

struct superblock {
    int size; /* size of blocks in bytes */
    int table_offset; /* offset of FAT table region in blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_block; /* head of free block list, index, if disk is full, -1 */
    char* padding;
};

    struct file_entry; 

    typedef struct file_entry { 

    char *name[8];

    u_int16_t FAT_entry; //first FAT entry, 2 bytes

    u_int32_t length; //legnth of file in bytes, 4 bytes

    u_int8_t uid; //owner's user ID

    u_int16_t protection; //9 protection bits 

    struct file_entry *next_file; //pointer to next file, if NULL, this is the last file in the directory

} file_entry;


//directory entry

typedef struct dir_entry{

    char *name[8];

    u_int16_t FAT_entry; //FAT entry for the directory

    u_int8_t uid; //owner's user ID

    u_int16_t protection; //9 protection bits 

    file_entry *head; //head of the file linked list (all the files in the directory)

    file_entry *tail; //tail of the linked list

} dir_entry;

struct dic{
    FILE* file;
    int file_ptr[N_BLOCKS]; 
};
struct superblock s_block;
u_int16_t table[4]; //eventually this will contain 4096 entries, but not right now!
file_entry r_block;
int test;
int format(char* filename, int dsize){
    r_block.FAT_entry = 0;
    r_block.next_file = NULL;
    //table[0] = r_block;
    for(int i = 0; i < 4; i++){
        table[i] = -1;
    }
    
    s_block.data_offset = 16;
    
    s_block.table_offset = 0; //from end of superblock. Change to 1 if instead the table offset is from beginning of file. 
    s_block.swap_offset = (512 * 4); //Is this actually needed? If the number is different, change it as you will. 
    s_block.free_block = 1953;
    s_block.padding = malloc(512 - sizeof(struct superblock));
    test = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    
    ftruncate(test, dsize * DISK_SIZE);
    write(test, &s_block, 512);
    close(test);
    free(s_block.padding);
}

int main(int argc, char* argv[]){
    s_block.size = 512;
    format(argv[1], 1);
    return 0;
}
