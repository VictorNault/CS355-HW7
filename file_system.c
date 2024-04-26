#include "common.h"
#include "file_system.h"

file_handle *open_files[NUM_OPEN_FILES]; //array for open files
FILE *disk;
fat_entry fat_table[TOTAL_BLOCKS];
int f_error = EXIT_SUCCESS;
int is_initialized = 0;
superblock *global_superblock;

struct current_block{
    char data[512];
};

char ** tokenize(const char * stringToSplit, int * cmdLen, char* delimiters){ 
    //counting the number of arguments passed by calling strtok twice (not the most efficient :()
    char * stringToSplitCopy = malloc( sizeof(char) * (strlen(stringToSplit)+1));
    strcpy(stringToSplitCopy, stringToSplit); 
    
    char * token = strtok(stringToSplitCopy,delimiters);
    *cmdLen = 0;
    while (token != NULL){
        (*cmdLen)++;
        token = strtok(NULL,delimiters);
    }

    if (*cmdLen == 0){ //nothing was passed
        return NULL;
    }

    char ** tokenList = (char **) malloc(sizeof(char *) * (*cmdLen));

    strcpy(stringToSplitCopy, stringToSplit); 
    token = strtok(stringToSplitCopy, delimiters);
    tokenList[0] = (char *) malloc(sizeof(char)* (strlen(token)+1));
    strcpy(tokenList[0],token);
    for (int i = 1; i < *cmdLen; i++){
        token = strtok(NULL, delimiters);
        tokenList[i] = (char *) malloc(sizeof(char)* (strlen(token)+1));
        strcpy(tokenList[i],token);
    }

    free(stringToSplitCopy);
    return tokenList;
}

unsigned long find_offset(int block){
    //finds the byte offset from 0 for the data block
    return global_superblock->data_offset * BLOCK_SIZE + block * BLOCK_SIZE;
}

fat_entry *find_next_fat(fat_entry *current){
    int index = current->next;
    if(index == -1){
        return NULL;
    }
    return &fat_table[index];
}

int find_file_from_directory(file_header *dir, fat_entry *fat, char *name){
    //returns 0 or 1 for a malloced file_header of the file we are trying to find
    dir_entry *cur_file = malloc(sizeof(dir_entry));
    fseek(disk,find_offset(dir->first_FAT_idx) + 16,SEEK_SET);
    fread(cur_file,sizeof(dir_entry),1,disk);
    fat_entry *cur_fat = fat;
    int total_size = 16;
    printf("name1: %s\n",cur_file->name);
    do{
        while(total_size < BLOCK_SIZE){
            total_size += sizeof(dir_entry);
            fread(cur_file,sizeof(dir_entry),1,disk);
            printf("name: %s\n",cur_file->name);
            if(strcmp(cur_file->name, name) == 0){
                printf("Successfully found file %s in directory %s\n",name,dir->name);
                fseek(disk,find_offset(cur_file->first_FAT_idx),SEEK_SET);
                fread(dir,sizeof(file_header),1,disk);
                free(cur_file);
                return EXIT_SUCCESS;
            }
        }
        if(cur_fat->next == -1){
            free(cur_file);
            printf("File %s not found in directory %s\n",name,dir->name);
            return EXIT_FAILURE;
        }
        fseek(disk,find_offset(cur_fat->next),SEEK_SET);
        fread(cur_file,sizeof(dir_entry),1,disk);
        cur_fat = find_next_fat(cur_fat);
        total_size = 0;
    }while(cur_fat->next != -1 && cur_fat->next != -2);

    free(cur_file);
    printf("File %s found in directory %s\n",name,dir->name);
    return EXIT_FAILURE;
}
    
void update_file_header(file_header *file_to_update){
    //updates the file entry in the corresponding FAT entry and the data block header
}

void f_terminate(){
    fclose(disk);
    is_initialized = 0;
}

void f_init(){ //initializing the disk
    if(!is_initialized){
        //reading disk
        disk = fopen("fresh_disk","rb+");

        //reading superblock
        global_superblock = malloc(sizeof(superblock));
        fread(global_superblock,sizeof(superblock),1,disk);
    
        //reading fat table
        fseek(disk,BLOCK_SIZE,SEEK_SET);
        fread(fat_table,TOTAL_BLOCKS*sizeof(fat_entry),1,disk);
        is_initialized = 1;
    }
    assert(disk);
    assert(is_initialized);
}

file_handle *f_open(const char *pathname, const int mode){
    //open a file with specified access mode
    //read, write, read/write, append
    //if file does not exist, create the file in the specified directory
    //returns file pointer if successful
    //opens a directory file for reading and returns a file_header
    
    file_header *file_e = malloc(sizeof(file_header));
    long cur_block = 0;

    //tokenizing the pathname
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    
    //seeking the root directory fat entry
    fat_entry *fat_e = &fat_table[0];

    //seeking the data block for root
    fseek(disk,find_offset(0),SEEK_SET);
    fread(file_e,sizeof(*file_e),1,disk);

    int status = 0;
    //finding file from directory repeatedly
    for(int i = 0; i < token_length; i++){
        status = find_file_from_directory(file_e,fat_e,tokens[i]);
        
        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            printf("FILE NOT FOUND\n");
            return NULL;
        }
        if(!file_e && i < token_length - 1){
            printf("Directory does not exist, exiting f_open\n");
            f_error = FILE_NOT_FOUND;
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            return NULL;
        }
        else if(!file_e && i == token_length - 1 && mode == READ_ONLY){
            printf("File does not exist in read mode, exiting f_open\n");
            f_error = FILE_NOT_FOUND;
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            return NULL;
        }
        // else if(!file_e && i == token_length - 1){
        //     printf("File does not exist, making file...\n");
        //     make_file();
        // }

        //update fat_entry
        fat_e = &fat_table[file_e->first_FAT_idx];
    }
    
    //making a new file handle
    file_handle *to_return = malloc(sizeof(file_handle));
    to_return->cur_rindex = 0; 
    to_return->cur_windex = 0;
    strcpy(to_return->name,file_e->name);
    to_return->size = file_e->size;
    to_return->first_FAT_idx = file_e->first_FAT_idx;

    //putting the file handle into the open_files array
    int status1 = 0;
    for(int i = 0; i < NUM_OPEN_FILES; i++){
        if(!open_files[i]){
            open_files[i] = to_return;
            status1 = 1;
            break;
        }
    }
    if(!status1){
        printf("Max number of open files reached\n");
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(tokens);
        free(file_e);
        return NULL;
    }

    //cleaning up
    for(int i = 0; i < token_length; i++){
        free(tokens[i]);
    }
    free(tokens);
    free(file_e);
    return to_return;
}

size_t f_read(void *ptr, size_t size, size_t nmemb, file_handle *stream){
    //read the specified number of bytes from a file handle at the current position. 
    //returns the number of bytes read, or an error.

    //the first block to read from
    int data_block_offset = (stream->cur_rindex + 16) / BLOCK_SIZE;
    fat_entry cur_fat_entry = fat_table[stream->first_FAT_idx];
    int cur_block = stream->first_FAT_idx;

    //setting the cur_fat_entry to that of the first block to read from
    for(int i = 0; i < data_block_offset; i++){
        cur_block = cur_fat_entry.next;
        cur_fat_entry = fat_table[cur_block];
    }




}

size_t f_write(const void *ptr, size_t size, size_t nmemb, file_handle *stream){
    //write some bytes to a file handle at the current position. 
    //Returns the number of bytes written, or an error.
}

int f_close(file_handle *stream){
    //close a file handle, cleans up memory in the open files list
}

int f_seek(file_handle *stream, long offset, int position){
    //struct current_block* cblock = malloc(sizeof(struct current_block));
    int block_num;
    int bit_num;
    if(position == SEEK_SET){
        block_num = offset / BLOCK_SIZE;
        bit_num = offset % BLOCK_SIZE;
        int base_bit = find_offset(block_num) + bit_num;
        //fread(cblock, 512, 1, disk);
        stream->cur_rchar = (char*)&base_bit;
        stream->cur_wchar = (char*)&base_bit;
        stream->cur_rindex = base_bit;
        stream->cur_windex = base_bit;
        return EXIT_SUCCESS;
    } 
}

// void f_rewind(file_handle *stream){
//     stream->cur_wchar = (char *)find_offset(file_e->first_FAT_idx)+32;
//     stream->cur_rchar = (char *)find_offset(file_e->first_FAT_idx)+32;
//     stream->cur_rindex = 0;
//     stream->cur_windex = 0;
//     //move pointers to the start of the file
// }

int f_stat(file_handle *stream, file_header *stat_buffer){
    //retrieve information about a file
    //updates the stat_buffer struct
}

int f_remove(file_handle *stream){
    //delete a file from disk
    //returns EXIT_SUCCESS if successfully deleted or error
}

// file_header *f_opendir(const char *pathname){
//     //opens a directory file for reading and returns a file_header
//     fat_entry *fat_e = malloc(sizeof(fat_entry));
//     file_header *file_e = malloc(sizeof(file_header));
//     long cur_block = 0;

//     //tokenizing the pathname
//     int token_length = 0;
//     char** tokens = tokenize(pathname,&token_length,"/");
    
//     //seeking the root directory fat entry
//     fseek(disk,find_offset(fat_offset),SEEK_SET); 
//     fread(fat_e,sizeof(*fat_e),1,disk);

//     //seeking the data block for root
//     fseek(disk,find_offset(cur_block),SEEK_SET);
//     fread(file_e,sizeof(*file_e),1,disk);

//     //finding file from directory repeatedly
//     for(int i = 0; i < token_length; i++){
//         file_e = find_file_from_directory(file_e,fat_e,tokens[i]);
//         //update fat_entry
//         if(!file_e){
//             printf("Directory not found, exiting f_opendir\n");
//             f_error = FILE_NOT_FOUND;
//             return NULL;
//         }
//         fat_e = &fat_table[file_e->first_FAT_idx];
//     }
//     return file_e;

//     //cleaning up
//     for(int i = 0; i < token_length; i++){
//         free(tokens[i]);
//     }
//     free(tokens);
//     return NULL;
// }

file_header *f_readdir(file_header *directory){
    //returns the next file_header in the directory, updates the pointer to the current file
}

int f_closedir(file_header *stream){
    //close an open directory file, cleans up memory in the open files list
}

int f_mkdir(const char *pathname, char *mode) {
    // need to update for situation where only one free block (head of list)
    //creates a new directory file in the specified location
    //make sure to update values for ./ and ../
    if (global_superblock->free_block == NONE_FREE) {
        printf("No free blocks, exiting f_mkdir\n");
        return EXIT_FAILURE;
    }
    

    //tokenizing the pathname 
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    char *name = malloc(sizeof(char)*9);
    strcpy(name,tokens[token_length-1]);
    printf("%s\n",name);
    if ((strlen(name) + 1) > NAME_BYTES) {
        printf("Name too long\n");
        return EXIT_FAILURE;
    }
    //seeking the root directory fat entry
    fat_entry *fat_e = &fat_table[0];
    file_header *file_e = malloc(sizeof(file_header));

    //seeking the data block for root
    fseek(disk,find_offset(0),SEEK_SET);
    fread(file_e,sizeof(*file_e),1,disk);

    int status = 0;
    //finding file from directory repeatedly
    for(int i = 0; i < token_length-1; i++){
        status = find_file_from_directory(file_e,fat_e,tokens[i]);
        
        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            printf("FILE NOT FOUND\n");
            return EXIT_FAILURE;
        }
    }//file_e should be the parent directory

    // +1 for null char
    int dir_block = file_e->first_FAT_idx;
    fseek(disk, find_offset(dir_block), SEEK_SET);
    dir_header *parent_dir = malloc(sizeof(dir_header));
    //fclose(global_rw_fp);
    //global_rw_fp = fopen(FAKEDISK_NAME, "rb+");
    fread(parent_dir, BLOCK_SIZE, 1, disk);

    // Get second free block,
    // Remove from free list,
    // Then fix free list
    free_datablock *head_of_free_list = malloc(sizeof(free_datablock));
    fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
    fread(head_of_free_list, BLOCK_SIZE, 1, disk);

    free_datablock *second_free_block = malloc(sizeof(free_datablock));
    fseek(disk, find_offset(head_of_free_list->next), SEEK_SET);
    fread(second_free_block, BLOCK_SIZE, 1, disk);

    // update and write free list
    int new_dir_block = head_of_free_list->next;
    head_of_free_list->next = second_free_block->next;
    fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
    fwrite(head_of_free_list, BLOCK_SIZE, 1, disk);

    // update and write fat table
    fat_entry new_fat_entry;
    new_fat_entry.next = -1;
    fat_table[new_dir_block] = new_fat_entry;
    fseek(disk, SUPERBLOCK_BYTES, SEEK_SET);
    fwrite(&fat_table, FATTABLE_BYTES, 1, disk);

    // add new block to parent dir (then write)
    //int non_header_bytes_in_parent = parent_dir.size - FILE_HEADER_BYTES;
    //parent_dir.data_in_first_block[non_header_bytes_in_parent] = new_dir_block;
    //fseek(global_rw_fp, BLOCK_BYTES * (DATA_OFFSET + dir_block), SEEK_SET);
    //fwrite(&parent_dir, BLOCK_BYTES, 1, global_rw_fp);

    // make dir_entry for new dir
    dir_entry *new_dir_entry = malloc(sizeof(dir_entry));
    strcpy(new_dir_entry->name, name);
    new_dir_entry->first_FAT_idx = new_dir_block;
    new_dir_entry->size = FILE_HEADER_BYTES + (2 * DIR_ENTRY_BYTES);
    new_dir_entry->uid = 101;
    new_dir_entry->protection[0] = TRUE;
    new_dir_entry->protection[1] = TRUE;
    new_dir_entry->protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        new_dir_entry->protection[i] = FALSE;
    }

    // add new block to parent dir (then write)
    int non_header_bytes_in_parent = parent_dir->size - FILE_HEADER_BYTES;
    int files_in_parent = non_header_bytes_in_parent / DIR_ENTRY_BYTES;
    parent_dir->data_in_first_block[files_in_parent] = *new_dir_entry;
    parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
    fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
    fwrite(parent_dir, BLOCK_SIZE, 1, disk);


    // finally make and write new dir itself
    dir_header *new_dir = malloc(sizeof(dir_header));
    strcpy(new_dir->name, name);
    new_dir->is_directory = TRUE;
    new_dir->first_FAT_idx = new_dir_block;
    new_dir->size = new_dir_entry->size;
    new_dir->data_in_first_block[0] = *new_dir_entry;
    new_dir->data_in_first_block[1] = parent_dir->data_in_first_block[0];
    fseek(disk, find_offset(new_dir_block), SEEK_SET);
    fwrite(new_dir, BLOCK_SIZE, 1, disk);
    free(new_dir);
    free(parent_dir);
    free(new_dir_entry);
    free(second_free_block);
    free(head_of_free_list);
    for(int i = 0; i < token_length; i++){
        free(tokens[i]);
    }
    free(tokens);
    free(file_e);
    free(name);
    return 0;
}

int f_rmdir(const char *pathname){
    //delete a directory, removes entire contents and the contents of all subdirectorys from the filesystem
}

int main(){
    f_init();
    f_mkdir("/next","e");
    f_mkdir("/hi","e");
    f_mkdir("/hiiii","e");
    dir_header temp;
    fseek(disk,find_offset(0),SEEK_SET);
    fread(&temp, BLOCK_SIZE, 1, disk);
    f_mkdir("/hi/wow","e");
    file_handle *test = f_open("/hiiii/",1);
    file_handle *test1 = f_open("/hi/wow/",1);
    // f_seek(test,2,SEEK_SET);


    // f_mkdir("/next/test","e");
    
    f_terminate();
    // int i = (1008+16) / 512;
    // printf("%d\n",i);
}
