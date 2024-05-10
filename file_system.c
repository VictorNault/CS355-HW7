#include "common.h"
#include "file_system.h"
#include "../FS-shell/string_extras.h"
#include "termios.h"
file_handle *open_files[NUM_OPEN_FILES]; //array for open files
FILE *disk;
fat_entry fat_table[TOTAL_BLOCKS];
int f_error = EXIT_SUCCESS;
int is_initialized = 0;
superblock *global_superblock;
int uid; //user id, 101 is the super user
char *prohibited_chars = "/<>;&";

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
        free(stringToSplitCopy);
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

void set_file_size(file_handle *fh, int n){
    //updates size of a file in three places, file handle, file header, and dir entry
    //updating file handle
    fh->size = n;

    //updating file header
    file_header *temp = malloc(sizeof(file_header));
    fseek(disk,find_offset(fh->first_FAT_idx),SEEK_SET);
    fread(temp,sizeof(file_header),1,disk);
    temp->size = n;
    fseek(disk,find_offset(fh->first_FAT_idx),SEEK_SET);
    fwrite(temp,sizeof(file_header),1,disk);
    free(temp);

    //updating dir entry
    int cur_fat_idx = fh->parent_FAT_idx;
    fat_entry cur_dir_fat = fat_table[cur_fat_idx];
    dir_header *temp1 = malloc(sizeof(dir_header));
    fseek(disk,find_offset(fh->parent_FAT_idx),SEEK_SET);
    fread(temp1,sizeof(dir_header),1,disk);
    //first block
    for(int i = 0; i < 15; i++){
        if(temp1->data_in_first_block[i].first_FAT_idx == fh->first_FAT_idx){
            temp1->data_in_first_block[i].size = n;
            fseek(disk,find_offset(fh->parent_FAT_idx),SEEK_SET);
            fwrite(temp1,sizeof(dir_header),1,disk);
            free(temp1);
            return;
        }
    }
    free(temp1);

    dir_entry *dir_entries = malloc(sizeof(dir_entry) * 16);
    cur_fat_idx = cur_dir_fat.next;
    //subsequent blocks for a multiblock dir header
    while(cur_fat_idx != -1){
        cur_dir_fat = fat_table[cur_fat_idx];
        
        fseek(disk,find_offset(cur_fat_idx),SEEK_SET);
        fread(dir_entries,BLOCK_SIZE,1,disk);
        for(int i = 0; i < 16; i++){
            if(dir_entries[i].first_FAT_idx == fh->first_FAT_idx){
                dir_entries[i].size = n;
                fseek(disk,find_offset(cur_fat_idx),SEEK_SET);
                fwrite(dir_entries,sizeof(dir_header),1,disk);
                free(dir_entries);
                return;
            }
        }
        cur_fat_idx = cur_dir_fat.next;
    }
    free(dir_entries);
    //printf("Didn't find dir_entry for the file in increase_file_size\n");
}

int find_file_from_directory(file_header *dir, fat_entry *fat, char *name, int *parent_dir_FAT_idx){
    //returns 0 or 1 for a malloced file_header of the file we are trying to find
    dir_entry *cur_file = malloc(sizeof(dir_entry));
    fseek(disk,find_offset(dir->first_FAT_idx) + FILE_HEADER_BYTES,SEEK_SET);
    fread(cur_file,sizeof(dir_entry),1,disk);
    int cur_idx = dir->first_FAT_idx;
    fat_entry *cur_fat = fat;
    int total_size = FILE_HEADER_BYTES;
    if (strcmp(name, ".") == 0){
        //printf("Successfully found file %s in directory %s\n",name,dir->name);
        if(parent_dir_FAT_idx){
            *parent_dir_FAT_idx = dir->first_FAT_idx;
        }
            fseek(disk,find_offset(cur_file->first_FAT_idx),SEEK_SET);
            fread(dir,sizeof(file_header),1,disk);
            free(cur_file);
            return EXIT_SUCCESS;
    }
    else if (strcmp(name, "..") == 0){
        total_size += sizeof(dir_entry);
        fread(cur_file,sizeof(dir_entry),1,disk);
        if(parent_dir_FAT_idx){
            *parent_dir_FAT_idx = dir->first_FAT_idx;
        }
        int offset = find_offset(cur_file->first_FAT_idx);
        fseek(disk,offset,SEEK_SET);
        fread(dir,sizeof(file_header),1,disk);
        free(cur_file);
        return EXIT_SUCCESS;    
    }
    do{
        while(total_size < BLOCK_SIZE){
            // printf("%s, %d\n",cur_file->name, total_size);
            total_size += sizeof(dir_entry);
            fread(cur_file,sizeof(dir_entry),1,disk);
            if(strcmp(cur_file->name, name) == 0){
                ////printf("Successfully found file %s in directory %s\n",name,dir->name);
                if(parent_dir_FAT_idx){
                    *parent_dir_FAT_idx = dir->first_FAT_idx;
                }
                int offset = find_offset(cur_file->first_FAT_idx);
                fseek(disk,offset,SEEK_SET);
                fread(dir,sizeof(file_header),1,disk);
                free(cur_file);
                return EXIT_SUCCESS;
            }
        }
        if(cur_idx == -1){
            free(cur_file);
            ////printf("File %s not found in directory %s\n",name,dir->name);
            return EXIT_FAILURE;
        }
        fseek(disk,find_offset(cur_fat->next),SEEK_SET);
        cur_idx = cur_fat->next;
        cur_fat = find_next_fat(cur_fat);
        total_size = -32; //-sizeof dir_entry
    }while(cur_idx != -1);
    free(cur_file);
    // //printf("File %s not found in directory %s\n",name,dir->name);
    return EXIT_FAILURE;
}
    
dir_entry *update_protection(int dir_FAT_idx, char *name, u_int8_t protection[]){
    //updates the protection bytes of a dir_entry, if protection is NULL just return the protection bytes
    dir_entry *to_return = malloc(sizeof(dir_entry));
    int cur_fat_idx = dir_FAT_idx;
    fat_entry cur_dir_fat = fat_table[cur_fat_idx];
    dir_header *temp1 = malloc(sizeof(dir_header));
    fseek(disk,find_offset(dir_FAT_idx),SEEK_SET);
    fread(temp1,sizeof(dir_header),1,disk);
    //first block
    for(int i = 0; i < 15; i++){
        if(strcmp(temp1->data_in_first_block[i].name,name) == 0){
            if(protection){
                for(int j = 0; j < 9; j++){ //copying the array
                    //printf("before: %d = %d \n", temp1->data_in_first_block[i].protection[j],  protection[j]);
                    temp1->data_in_first_block[i].protection[j] = protection[j];
                    //printf("after : %d = %d \n", temp1->data_in_first_block[i].protection[j],  protection[j]);
                }
                fseek(disk,find_offset(dir_FAT_idx),SEEK_SET);
                fwrite(temp1,sizeof(dir_header),1,disk);
            }
            *to_return = temp1->data_in_first_block[i];
            free(temp1);
            return to_return;
        }
    }
    free(temp1);

    dir_entry *dir_entries = malloc(sizeof(dir_entry) * 16);
    cur_fat_idx = cur_dir_fat.next;
    //subsequent blocks for a multiblock dir header
    while(cur_fat_idx != -1){
        cur_dir_fat = fat_table[cur_fat_idx];
        
        fseek(disk,find_offset(cur_fat_idx),SEEK_SET);
        fread(dir_entries,BLOCK_SIZE,1,disk);
        for(int i = 0; i < 16; i++){
            if(strcmp(dir_entries[i].name,name) == 0){
                if(protection){
                    for(int i = 0; i < 9; i++){ //copying the array
                        dir_entries[i].protection[i] = protection[i];
                    }
                    fseek(disk,find_offset(cur_fat_idx),SEEK_SET);
                    fwrite(temp1,sizeof(dir_header),1,disk);
                }
                *to_return = dir_entries[i];
                free(dir_entries);
                return to_return;
            }
        }
        cur_fat_idx = cur_dir_fat.next;
    }
    free(dir_entries);
    free(to_return);
    return NULL;
}

int add_block_to_file(fat_entry *last_fat_entry){
    //append a new block to the file, returns the index of the new block
    //returns -1 if error
    if(global_superblock->free_block == -1){
        //printf("No free block left, exiting...\n");
        f_error = E_NO_SPACE;
        return -1;
    }
    //int second_free_idx = fat_table[global_superblock->free_block].next;
    free_datablock *cur_free_block = malloc(sizeof(free_datablock));
    fseek(disk,find_offset(global_superblock->free_block),SEEK_SET);
    fread(cur_free_block,BLOCK_SIZE,1,disk);
    int second_free_idx = cur_free_block->next;
    if(second_free_idx == -1){
        //only one free block left, update superblock and write to disk
        int free_block = global_superblock->free_block;
        fat_table[free_block].next = -1;
        global_superblock->free_block = -1;
        last_fat_entry->next = free_block;
        fseek(disk,0,0);
        fwrite(global_superblock,sizeof(superblock),1,disk);
        //Writing entire fat table to disk
        fseek(disk,BLOCK_SIZE,SEEK_SET);
        fwrite(fat_table,TOTAL_BLOCKS*sizeof(fat_entry),1,disk);
        //printf("Allocating the last free block %d to file\n",free_block);
        free(cur_free_block);
        return free_block;
    }
    fat_entry *first_free_fat = &fat_table[global_superblock->free_block];
    fat_entry *second_free_fat = &fat_table[second_free_idx];
    //printf("Allocating a new block to the file, second free block is block %d\n",second_free_idx);
    
    //updating the tail fat_entry and free list
    last_fat_entry->next = second_free_idx;
    //first_free_fat->next = second_free_fat->next;
    free_datablock *next_free_block = malloc(sizeof(free_datablock));
    fseek(disk,find_offset(cur_free_block->next),SEEK_SET);
    fread(next_free_block,BLOCK_SIZE,1,disk);
    cur_free_block->next = next_free_block->next;
    second_free_fat->next = -1;
    
    //writing free blocks that are modified to disk
    fseek(disk,find_offset(global_superblock->free_block),SEEK_SET);
    fwrite(cur_free_block,BLOCK_SIZE,1,disk);

    //Writing entire fat table to disk
    fseek(disk,BLOCK_SIZE,SEEK_SET);
    fwrite(fat_table,TOTAL_BLOCKS*sizeof(fat_entry),1,disk);
    
    free(cur_free_block);
    free(next_free_block);
    return second_free_idx;
}

void f_terminate(){
    fclose(disk);
    is_initialized = 0;
    free(global_superblock);
}

void f_init(int user_id, char *disk_name){ //initializing the disk
    if(!is_initialized){
        //reading disk
        disk = fopen(disk_name,"rb+");
        uid = user_id;
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

int f_mkdir(const char *pathname, char *mode) {
    // need to update for multiblock dir support
    //creates a new directory file in the specified location
    if (global_superblock->free_block == NONE_FREE) {
        //printf("No free blocks, exiting f_mkdir\n");
        f_error = E_NO_SPACE;
        return EXIT_FAILURE;
    }
    
    //tokenizing the pathname 
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    char *name = malloc(sizeof(char)*9);
    strcpy(name,tokens[token_length-1]);
    if ((strlen(name) + 1) > NAME_BYTES) {
        //printf("Name too long, existing f_mkdir\n");
        f_error = E_BAD_NAME;
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(name);
        free(tokens);
        return EXIT_FAILURE;
    }

    //check for the integrity of name
    if(inStr(name,prohibited_chars)){
        //printf("Name '%s' contains prohibit chars '%s', existing f_mkdir\n",name,prohibited_chars);
        f_error = E_BAD_NAME;
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(name);
        free(tokens);
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
        status = find_file_from_directory(file_e,fat_e,tokens[i],NULL);
        
        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(name);
            free(tokens);
            free(file_e);
            //printf("FILE NOT FOUND, existing f_mkdir\n");
            return EXIT_FAILURE;
        }
    }//file_e should be the parent directory

    // +1 for null char
    int dir_block = file_e->first_FAT_idx;

    //checking if a file with the same name exists in parent directory
    if(find_file_from_directory(file_e,fat_e,name,NULL) == EXIT_SUCCESS){
        //printf("File with name '%s' exists in parent directory, exiting f_mkdir\n",name);
        f_error = E_BAD_NAME;
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(name);
        free(tokens);
        free(file_e);
        return EXIT_FAILURE;
    }

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
    int new_dir_block;
    if (head_of_free_list->next != NONE_FREE) {
        fseek(disk, find_offset(head_of_free_list->next), SEEK_SET);
        fread(second_free_block, BLOCK_SIZE, 1, disk);

        // update and write free list
        new_dir_block = head_of_free_list->next;
        head_of_free_list->next = second_free_block->next;
        fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
        fwrite(head_of_free_list, BLOCK_SIZE, 1, disk);
    }
    else {
        new_dir_block = global_superblock->free_block;
        global_superblock->free_block = NONE_FREE;
        fseek(disk, 0, SEEK_SET);
        fwrite(global_superblock, BLOCK_SIZE, 1, disk);
    }

    // update and write fat table
    fat_entry new_fat_entry;
    new_fat_entry.next = NONE_FREE;
    fat_table[new_dir_block] = new_fat_entry;
    fseek(disk, SUPERBLOCK_BYTES, SEEK_SET);
    fwrite(&fat_table, FATTABLE_BYTES, 1, disk);

    // make dir_entry for new dir
    dir_entry *new_dir_entry = malloc(sizeof(dir_entry));
    strcpy(new_dir_entry->name, name);
    new_dir_entry->first_FAT_idx = new_dir_block;
    new_dir_entry->size = FILE_HEADER_BYTES + (2 * DIR_ENTRY_BYTES);
    new_dir_entry->uid = uid;
    new_dir_entry->protection[0] = TRUE;
    new_dir_entry->protection[1] = TRUE;
    new_dir_entry->protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        new_dir_entry->protection[i] = FALSE;
    }
    new_dir_entry->is_directory = TRUE;

    // add new block to parent dir (then write)
    int non_header_bytes_in_parent = parent_dir->size - FILE_HEADER_BYTES;
    int files_in_parent = non_header_bytes_in_parent / DIR_ENTRY_BYTES;
    ////printf("Non_header_bytes: %d\nfiles_in_parent: %d\n",non_header_bytes_in_parent,files_in_parent);
    if(files_in_parent >= 15){
        //parent dir is full, needs to check if the next block exists or allocate new block
        int cur_idx = parent_dir->first_FAT_idx;
        fat_entry* cur_fat = &fat_table[cur_idx];
        while(cur_fat->next != -1){ //getting the last fat
            cur_idx = cur_fat->next;
            cur_fat = &fat_table[cur_idx];
        }
        dir_entry *dir_entries = malloc(BLOCK_SIZE); //getting the last dir entries block
        fseek(disk,find_offset(cur_idx),SEEK_SET);
        fread(dir_entries,BLOCK_SIZE,1,disk);
        //printf("Size: %d\n",parent_dir->size);
        if(((parent_dir->size)/32)%16 != 0){
            //if there is an empty spot in the last block, write to the last spot
            dir_entries[((parent_dir->size)/32)%16] = *new_dir_entry;
            parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
            fseek(disk,find_offset(cur_idx),SEEK_SET);
            fwrite(dir_entries,BLOCK_SIZE,1,disk);
            free(dir_entries);
        }else{
            //if everything is full, create a new block and put the new dir entry in the first spot
            int idx = add_block_to_file(cur_fat);
            fseek(disk,find_offset(idx),SEEK_SET);
            fread(dir_entries,BLOCK_SIZE,1,disk);
            dir_entries[0] = *new_dir_entry;
            parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
            fseek(disk,find_offset(idx),SEEK_SET);
            fwrite(dir_entries,BLOCK_SIZE,1,disk);
            free(dir_entries);
        }
        fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
        fwrite(parent_dir, BLOCK_SIZE, 1, disk);
    }else{
        parent_dir->data_in_first_block[files_in_parent] = *new_dir_entry;
        parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
        fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
        fwrite(parent_dir, BLOCK_SIZE, 1, disk);
    }
    


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

int f_mkfile(const char *pathname, char *mode) {
    // need to update for situation where only one free block (head of list)
    //creates a new directory file in the specified location
    //make sure to update values for ./ and ../
    if (global_superblock->free_block == NONE_FREE) {
        //printf("No free blocks, exiting f_mkdir\n");
        return EXIT_FAILURE;
    }
    

    //tokenizing the pathname 
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    char *name = malloc(sizeof(char)*9);
    strcpy(name,tokens[token_length-1]);
    if ((strlen(name) + 1) > NAME_BYTES) {
        //printf("Name too long, exiting f_mkfile...\n");
        f_error = E_BAD_NAME;
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(name);
        free(tokens);
        return EXIT_FAILURE;
    }

     //check for the integrity of name
    if(inStr(name,prohibited_chars)){
        //printf("Name '%s' contains prohibit chars: %s, existing f_mkfile\n",name,prohibited_chars);
        f_error = E_BAD_NAME;
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(name);
        free(tokens);
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
        status = find_file_from_directory(file_e,fat_e,tokens[i],NULL);
        
        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            //printf("FILE NOT FOUND\n");
            return EXIT_FAILURE;
        }
    }//file_e should be the parent directory

    // +1 for null char
    int file_block = file_e->first_FAT_idx;

    //checking if a file with the same name exists in parent directory
    if(find_file_from_directory(file_e,fat_e,name,NULL) == EXIT_SUCCESS){
        //printf("File with name '%s' exists in parent directory, exiting f_mkfile\n",name);
        f_error = E_BAD_NAME;
        for(int i = 0; i < token_length; i++){
            free(tokens[i]);
        }
        free(name);
        free(tokens);
        free(file_e);
        return EXIT_FAILURE;
    }

    fseek(disk, find_offset(file_block), SEEK_SET);
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
    int new_file_block;
    if (head_of_free_list->next != NONE_FREE) {
        fseek(disk, find_offset(head_of_free_list->next), SEEK_SET);
        fread(second_free_block, BLOCK_SIZE, 1, disk);

        // update and write free list
        new_file_block = head_of_free_list->next;
        head_of_free_list->next = second_free_block->next;
        fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
        fwrite(head_of_free_list, BLOCK_SIZE, 1, disk);
    }
    else {
        new_file_block = global_superblock->free_block;
        global_superblock->free_block = NONE_FREE;
        fseek(disk, 0, SEEK_SET);
        fwrite(global_superblock, BLOCK_SIZE, 1, disk);
    }

    // update and write fat table
    fat_entry new_fat_entry;
    new_fat_entry.next = NONE_FREE;
    fat_table[new_file_block] = new_fat_entry;
    fseek(disk, SUPERBLOCK_BYTES, SEEK_SET);
    fwrite(&fat_table, FATTABLE_BYTES, 1, disk);

    // make dir_entry for new file
    dir_entry *new_dir_entry = malloc(sizeof(dir_entry));
    strcpy(new_dir_entry->name, name);
    new_dir_entry->first_FAT_idx = new_file_block;
    new_dir_entry->size = FILE_HEADER_BYTES;
    new_dir_entry->uid = 101;
    new_dir_entry->protection[0] = TRUE;
    new_dir_entry->protection[1] = TRUE;
    new_dir_entry->protection[2] = TRUE;
    for (int i = 3; i < 10; i++) {
        new_dir_entry->protection[i] = FALSE;
    }
    new_dir_entry->is_directory = FALSE;

    // add new block to parent dir (then write)
    int non_header_bytes_in_parent = parent_dir->size - FILE_HEADER_BYTES;
    int files_in_parent = non_header_bytes_in_parent / DIR_ENTRY_BYTES;
    //printf("Non_header_bytes: %d\nfiles_in_parent: %d\n",non_header_bytes_in_parent,files_in_parent);
    if(files_in_parent >= 15){
        //parent dir is full, needs to check if the next block exists or allocate new block
        int cur_idx = parent_dir->first_FAT_idx;
        fat_entry* cur_fat = &fat_table[cur_idx];
        while(cur_fat->next != -1){ //getting the last fat
            cur_idx = cur_fat->next;
            cur_fat = &fat_table[cur_idx];
        }
        dir_entry *dir_entries = malloc(BLOCK_SIZE); //getting the last dir entries block
        fseek(disk,find_offset(cur_idx),SEEK_SET);
        fread(dir_entries,BLOCK_SIZE,1,disk);
        //printf("Size: %d\n",parent_dir->size);
        if(((parent_dir->size)/32)%16 != 0){
            //if there is an empty spot in the last block, write to the last spot
            dir_entries[((parent_dir->size)/32)%16] = *new_dir_entry;
            parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
            fseek(disk,find_offset(cur_idx),SEEK_SET);
            fwrite(dir_entries,BLOCK_SIZE,1,disk);
            free(dir_entries);
        }else{
            //if everything is full, create a new block and put the new dir entry in the first spot
            int idx = add_block_to_file(cur_fat);
            fseek(disk,find_offset(idx),SEEK_SET);
            fread(dir_entries,BLOCK_SIZE,1,disk);
            dir_entries[0] = *new_dir_entry;
            parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
            fseek(disk,find_offset(idx),SEEK_SET);
            fwrite(dir_entries,BLOCK_SIZE,1,disk);
            free(dir_entries);
        }
        fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
        fwrite(parent_dir, BLOCK_SIZE, 1, disk);
        if (parent_dir->first_FAT_idx != 0){
        file_handle dum;
        dum.parent_FAT_idx = parent_dir->data_in_first_block[1].first_FAT_idx;
        dum.first_FAT_idx = parent_dir->first_FAT_idx;
        set_file_size(&dum,parent_dir->size);
        }
    }else{
        parent_dir->data_in_first_block[files_in_parent] = *new_dir_entry;
        parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
        fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
        fwrite(parent_dir, BLOCK_SIZE, 1, disk);
        if (parent_dir->first_FAT_idx != 0){
        file_handle dum; // grand parent 
        dum.parent_FAT_idx = parent_dir->data_in_first_block[1].first_FAT_idx;
        dum.first_FAT_idx = parent_dir->first_FAT_idx;
        set_file_size(&dum,parent_dir->size);
        }
        
    }


    // finally make and write new file itself
    file_header *new_file = malloc(sizeof(file_header));
    strcpy(new_file->name, name);
    new_file->is_directory = FALSE;
    new_file->first_FAT_idx = new_file_block;
    new_file->size = new_dir_entry->size;
    fseek(disk, find_offset(new_file_block), SEEK_SET);
    fwrite(new_file, BLOCK_SIZE, 1, disk);
    free(new_file);
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
    int parent_FAT_idx = 0;
    //finding file from directory repeatedly
    for(int i = 0; i < token_length; i++){
        status = find_file_from_directory(file_e,fat_e,tokens[i],&parent_FAT_idx);
        
        //error checking
        if(status == EXIT_FAILURE && i < token_length - 1){
            //printf("Directory does not exist, exiting f_open\n");
            f_error = E_FILE_NOT_FOUND;
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            return NULL;
        }
        else if(status == EXIT_FAILURE && i == token_length - 1 && mode == READ_ONLY){
            //printf("File %s does not exist in read mode, exiting f_open\n",file_e->name);
            f_error = E_FILE_NOT_FOUND;
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            return NULL;
        }
        else if(status == EXIT_FAILURE && i == token_length - 1){
            //printf("File does not exist, making file...\n");
            f_mkfile(pathname,"e");
            status = find_file_from_directory(file_e,fat_e,tokens[i],&parent_FAT_idx);
        }

        //update fat_entry
        fat_e = &fat_table[file_e->first_FAT_idx];
    }

    //***check if open files already contains the same file, then don't open
    for(int i = 0; i < NUM_OPEN_FILES; i++){
        if(open_files[i] && open_files[i]->first_FAT_idx == file_e->first_FAT_idx){
            //printf("File %s already open, exiting f_open...\n", file_e->name);
            f_error = E_FILE_ALREADY_OPEN;
            return NULL;
        }
    }   
    
    //making a new file handle
    file_handle *to_return = malloc(sizeof(file_handle));
    to_return->cur_rindex = 0; 
    to_return->cur_windex = 0;
    to_return->is_dir = 0;
    to_return->parent_FAT_idx = parent_FAT_idx;
    strcpy(to_return->name,file_e->name);
    to_return->size = file_e->size;
    to_return->first_FAT_idx = file_e->first_FAT_idx;

    //adjusting the file handle based on mode
    if(mode == READ_ONLY){
        to_return->cur_windex = -1;
    }else if(mode == WRITE_ONLY){
        to_return->cur_rindex = -1;
    }else if(mode == APPEND){
        to_return->cur_windex = file_e->size - 32;   
        to_return->cur_windex = to_return->cur_windex - 32;
    }
    
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
        //printf("Max number of open files reached\n");
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
    if (stream == NULL || stream->is_dir){
        //printf("Invalid Stream, exiting f_read...\n");
        f_error = E_NOT_FILE;
        return 0;
    }
    //check if we have permission to read
    if(stream->cur_rindex == -1){
        //printf("No read permission, exiting f_read...\n");
        f_error = E_PERMISSION_DENIED;
        return 0;
    }

    //check for user permission
    if(uid != 101){ //101 is super user
        dir_entry *my_de = update_protection(stream->parent_FAT_idx,stream->name,NULL);
        if(uid == my_de->uid){
            if(!my_de->protection[0]){
                //printf("No read permission, exiting f_read...\n");
                f_error = E_PERMISSION_DENIED;
                return 0;
            }
        }else if(!my_de->protection[6]){
            //printf("No read permission, exiting f_read...\n");
            f_error = E_PERMISSION_DENIED;
            return 0;
        }
        free(my_de);
    }

    //the first block to read from
    int data_block_offset = (stream->cur_rindex + FILE_HEADER_BYTES) / BLOCK_SIZE;
    fat_entry cur_fat_entry = fat_table[stream->first_FAT_idx];
    int cur_block = stream->first_FAT_idx;

    //setting the cur_fat_entry to that of the first block to read from
    for(int i = 0; i < data_block_offset; i++){
        cur_block = cur_fat_entry.next;
        if(cur_block == -1){
            //printf("Read out of bounds\n");
            f_error = E_OUT_OF_BOUNDS;
            return 0;
        }
        cur_fat_entry = fat_table[cur_block];
        
    }

    //variables for reading
    size_t total_size = size * nmemb;
    int bytes_to_read = 0;
    int copy_offset = 0;
    int multi_block_read = 0;
    ////printf("data_block_offset: %d\ncur_block: %d\nbytes_to_read: %d\n",data_block_offset,cur_block,bytes_to_read);
    while(total_size >= 0 && (stream->cur_rindex + FILE_HEADER_BYTES < stream->size)){
        //setting how many bytes I'm reading in this current block
        bytes_to_read = BLOCK_SIZE - ((stream->cur_rindex + FILE_HEADER_BYTES) % BLOCK_SIZE);
        if(total_size < bytes_to_read){
            bytes_to_read = total_size;
        }

        void *buffer = malloc(bytes_to_read);

        //reading from the current block
        int disk_offset = find_offset(cur_block) + ((stream->cur_rindex + FILE_HEADER_BYTES) % BLOCK_SIZE);
        // if(cur_block == stream->first_FAT_idx){
        //     disk_offset += FILE_HEADER_BYTES;
        // }
        // if(multi_block_read){
        //     disk_offset = disk_offset - stream->cur_rindex % BLOCK_SIZE;
        // }
        // //printf("cur_block: %d\ndisk_offset:%d\n",cur_block,disk_offset);
        fseek(disk,disk_offset,SEEK_SET);
        fread(buffer,bytes_to_read,1,disk);

        //copying what I read into the pointer from user
        memcpy(ptr + copy_offset,buffer,bytes_to_read);
        copy_offset += bytes_to_read;
        total_size -= bytes_to_read;
        stream->cur_rindex += bytes_to_read;
        if(total_size == 0){
            free(buffer);
            return copy_offset;
        }
        //going to the next data block
        cur_block = cur_fat_entry.next;
        multi_block_read = 1;
        if(cur_block == -1){
            //printf("Read out of bounds\n");
            f_error = E_OUT_OF_BOUNDS;
            free(buffer);
            return copy_offset;
        }
        cur_fat_entry = fat_table[cur_block];
        free(buffer);
    }
    //bandaid?
    if ((stream->cur_rindex + FILE_HEADER_BYTES >= stream->size)){
        return  copy_offset;
    }
    assert(total_size == 0);
    assert(copy_offset == size * nmemb);

    return copy_offset;
}

size_t f_write(const void *ptr, size_t size, size_t nmemb, file_handle *stream){
    //write some bytes to a file handle at the current position. 
    //Returns the number of bytes written, or an error.
    if (stream == NULL || stream->is_dir){
        //printf("Invalid Stream, exiting f_write...\n");
        f_error = E_NOT_FILE;
        return 0;
    }
    //check if we have permission to write

    if(stream->cur_windex == -1){
        //printf("No write permission, exiting f_write...\n");
        f_error = E_PERMISSION_DENIED;
        return 0;
    }

    //check for user permission
    if(uid != 101){ //101 is super user
        dir_entry *my_de = update_protection(stream->parent_FAT_idx,stream->name,NULL);
        if(uid == my_de->uid){
            if(!my_de->protection[1]){
                //printf("No write permission, exiting f_write...\n");
                f_error = E_PERMISSION_DENIED;
                return 0;
            }
        }else if(!my_de->protection[7]){
            //printf("No write permission, exiting f_write...\n");
            f_error = E_PERMISSION_DENIED;
            return 0;
        }
        free(my_de);
    }

    //the first block to write to
    int data_block_offset = (stream->cur_windex + FILE_HEADER_BYTES) / BLOCK_SIZE;
    fat_entry *cur_fat_entry = &fat_table[stream->first_FAT_idx];
    int cur_block = stream->first_FAT_idx;
    int multiblock_write = 0;
    size_t total_size = size * nmemb;
    //setting the cur_fat_entry to that of the first block to read from
    for(int i = 0; i < data_block_offset; i++){
        cur_block = cur_fat_entry->next;
        if(cur_block == -1){
            //need to add a block to the end of the file
            cur_block = add_block_to_file(cur_fat_entry);
            if(cur_block == -1){ //no space
                return 0;
            }
            //modifying size of file
            if(i < data_block_offset - 1){
                set_file_size(stream,stream->size + BLOCK_SIZE);
            }else if(i == data_block_offset - 1){
                set_file_size(stream,stream->cur_windex + FILE_HEADER_BYTES);
            }
            multiblock_write = 1;
        }
        cur_fat_entry = &fat_table[cur_block];
        
    }
    assert(stream->size >= stream->cur_windex);

    //variables for writing
    
    int bytes_to_write = 0;
    int copy_offset = 0;
    
    while(total_size >= 0){
        //setting how many bytes I'm reading in this current block
        bytes_to_write = BLOCK_SIZE - ((stream->cur_windex + FILE_HEADER_BYTES) % BLOCK_SIZE);
        if(total_size < bytes_to_write){
            bytes_to_write = total_size;
        }

        //writing to the current block
        int disk_offset = find_offset(cur_block) + ((stream->cur_windex + FILE_HEADER_BYTES) % BLOCK_SIZE);
        // if(cur_block == stream->first_FAT_idx){
        //     disk_offset += FILE_HEADER_BYTES;
        // }
        // if(multiblock_write){
        //     disk_offset = disk_offset - stream->cur_windex % BLOCK_SIZE;
        // }
        //printf("cur_block: %d\ndisk_offset mod 512:%d, writing %s\n",cur_block,disk_offset % 512, (char*)ptr+copy_offset);

        fseek(disk,disk_offset,SEEK_SET);
        fwrite(ptr + copy_offset,bytes_to_write,1,disk);

        
        copy_offset += bytes_to_write;
        total_size -= bytes_to_write;
        stream->cur_windex += bytes_to_write;

        //printf("windex = %ld streamsize = %ld, bytestowrite = %d\n", stream->cur_windex, stream->size, bytes_to_write); 
        //updating size
        if(stream->cur_windex > stream->size - FILE_HEADER_BYTES){
            set_file_size(stream,stream->cur_windex + FILE_HEADER_BYTES);
        }
        if(total_size == 0){
            break; //break out the loop if we are done
        }

        //going to the next data block
        cur_block = cur_fat_entry->next;
        multiblock_write = 1;
        if(cur_block == -1){
            //need to add a block to the end of the file
            cur_block = add_block_to_file(cur_fat_entry);
            if(cur_block == -1){ //no space
                return copy_offset;
            }
            if(total_size < BLOCK_SIZE){
                set_file_size(stream,stream->size + total_size);
            }else{
                set_file_size(stream,stream->size + BLOCK_SIZE);
            }
        }
        cur_fat_entry = &fat_table[cur_block];
    }
    
    assert(total_size == 0);
    assert(copy_offset == size * nmemb);
    return copy_offset;
}

int f_close(file_handle *stream){
    //free the memory of the file handle, cleans up the open files list by setting the corresponding index to NULL
    if(!stream){
        return EXIT_FAILURE;
    }
    for(int i = 0; i < NUM_OPEN_FILES; i++){
        if(open_files[i] && open_files[i]->first_FAT_idx == stream->first_FAT_idx){
            //found the file
            open_files[i] = NULL;
        }
    }   
    free(stream);
    return EXIT_SUCCESS;
}

int f_seek(file_handle *stream, long offset, int position){
    //moving r and w indexes in the file handle

    if(position == SEEK_SET){
        if(stream->cur_rindex != -1){
            stream->cur_rindex = offset;
        }
        if(stream->cur_windex != -1){
            stream->cur_windex = offset;
        }
        return EXIT_SUCCESS;
    } 
    else if(position == SEEK_CUR){   
        if(stream->cur_rindex != -1){
            stream->cur_rindex += offset;
        }
        if(stream->cur_windex != -1){
            stream->cur_windex += offset;
        }
        return EXIT_SUCCESS;
    } else if(position == SEEK_END){
        if(stream->cur_rindex != -1){
            stream->cur_rindex = (TOTAL_BYTES - offset);
        }
        if(stream->cur_windex != -1){
            stream->cur_windex = (TOTAL_BYTES - offset);
        }
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
    //move pointers to a specified position in a file
}

void f_rewind(file_handle *stream){
    if(stream->cur_rindex != -1){
        stream->cur_rindex = 0;
    }
    if(stream->cur_windex != -1){
        stream->cur_windex = 0;
    }
    //move pointers to the start of the file
}

int f_stat(file_handle *stream, file_stat *stat_buffer){
    stat_buffer->first_FAT_idx = stream->first_FAT_idx;
    stat_buffer->is_dir = stream->is_dir;
    stat_buffer->size = stream->size;
    strcpy(stat_buffer->name, stream->name);
    dir_entry *temp = update_protection(stream->parent_FAT_idx,stream->name,NULL);
    if(!temp){
        printf("File not found, exiting f_stat...\n");
        f_error = E_FILE_NOT_FOUND;
        free(temp);
        return EXIT_FAILURE;
    }else{
        stat_buffer->uid = temp->uid;
        for(int j = 0; j < 9; j++){ //copying the array
            stat_buffer->protection[j] = temp->protection[j];
        }
    }
    free(temp);
    return EXIT_SUCCESS;
}

void aux_insert_into_freelist(int fat_index_of_to_insert) {
    if (global_superblock->free_block == NONE_FREE) {
        free_datablock new_first_free_db;
        new_first_free_db.next = -1;
        global_superblock->free_block = fat_index_of_to_insert;
        fseek(disk, find_offset(fat_index_of_to_insert), SEEK_SET);
        fwrite(&new_first_free_db, BLOCK_SIZE, 1, disk);
        fseek(disk, 0, SEEK_SET);
        fwrite(global_superblock, BLOCK_SIZE, 1, disk);
    }

}

int f_remove(const char *pathname){
    //delete a file from disk
    //returns EXIT_SUCCESS if successfully deleted or error
    //tokenizing the pathname 
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    char *name = malloc(sizeof(char)*9);
    strcpy(name,tokens[token_length-1]);
    if ((strlen(name) + 1) > NAME_BYTES) {
        //printf("Name too long\n");
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
        status = find_file_from_directory(file_e,fat_e,tokens[i],NULL);
        
        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            //printf("FILE NOT FOUND\n");
            return EXIT_FAILURE;
        }
    }//file_e should be the parent directory

    // +1 for null char
    int dir_block = file_e->first_FAT_idx;
    fseek(disk, find_offset(dir_block), SEEK_SET);
    dir_header *parent_dir = malloc(sizeof(dir_header));
    fread(parent_dir, BLOCK_SIZE, 1, disk);



    // get correct dir entry
    int dir_entries_counter = 0;

    /*
    // cut last file off path
    // need null char!
    char path_cpy[strlen(pathname) + 1];
    strcpy(path_cpy, pathname);
    char * last_file;
    last_file = strrchr(path_cpy, '/');
    int bytes_to_copy = (int) (last_file - path_cpy);
    // need +1 for null character
    char new_path[bytes_to_copy + 1];
    strncpy(new_path, path_cpy, (size_t) bytes_to_copy);
    new_path[bytes_to_copy] = '\0';
    */

    // look for file to delete
    int files_in_parent_dir = (parent_dir->size - FILE_HEADER_BYTES) / DIR_ENTRY_BYTES;
    // initialize to data after header
    int possible_files_left_in_block = FILE_AFTER_HEADER_BYTES / DIR_ENTRY_BYTES;
    dir_entry curr_block_ar[BLOCK_SIZE / DIR_ENTRY_BYTES];
    for (int i = 0; i < possible_files_left_in_block; i++) {
        curr_block_ar[i] = parent_dir->data_in_first_block[i];
    }

    int to_delete_file_index = -1;
    int to_delete_block_ar_index = -1;
    dir_entry curr_dir_entry;
    int ar_counter = 0;
    int block_to_modify_fat_index = -1;
    fat_entry curr_fat_entry = fat_table[parent_dir->first_FAT_idx];
    dir_entry to_delete_dir_entry;
    int curr_fat_index = parent_dir->first_FAT_idx;
    for (int i = 0; i < files_in_parent_dir; i++) {
        if (possible_files_left_in_block == 0) {
            possible_files_left_in_block = BLOCK_SIZE / DIR_ENTRY_BYTES;
            ar_counter = 0;
            // get next block
            fseek(disk, find_offset(curr_fat_entry.next), SEEK_SET);
            fread(curr_block_ar, BLOCK_SIZE, 1, disk);
            curr_fat_index = curr_fat_entry.next;
            curr_fat_entry = fat_table[curr_fat_entry.next];
        }
        curr_dir_entry = curr_block_ar[ar_counter];
        if (strcmp(name, curr_dir_entry.name) == 0) {
            to_delete_file_index = i;
            to_delete_block_ar_index = ar_counter;
            to_delete_dir_entry = curr_dir_entry;
            block_to_modify_fat_index = curr_fat_index;
            if (curr_dir_entry.is_directory == TRUE) {
                // need "not file" error
                //f_error = 
                return EXIT_FAILURE;
            }
        }
        --possible_files_left_in_block;
        ++ar_counter;
    }

    // if file to delete is not found
    if (to_delete_file_index == -1) {
        f_error = E_FILE_NOT_FOUND;
        return EXIT_FAILURE;
    }

    dir_entry deleted_dir_entry;
    strcpy(deleted_dir_entry.name, "DELETED!");
    deleted_dir_entry.first_FAT_idx = UNUSED_BLOCK;
    deleted_dir_entry.size = -1;
    deleted_dir_entry.uid = 101;
    for (int i = 0; i < 10; i++) {
        deleted_dir_entry.protection[i] = FALSE;
    }
    deleted_dir_entry.is_directory = FALSE;

    //dir_entry swap_dir_entry;
    dir_entry block_to_modify_ar[BLOCK_SIZE / DIR_ENTRY_BYTES];

    // case 1: block containing file to delete does not contain parent dir header, parent dir is multiblock
    if ((curr_fat_index != parent_dir->first_FAT_idx) && (block_to_modify_fat_index != parent_dir->first_FAT_idx)) {
        // check if file entry to delete is at end of dir entries
        if (to_delete_file_index == (files_in_parent_dir - 1)) {
            // if so just delete the last entry in the last block
            curr_block_ar[to_delete_block_ar_index] = deleted_dir_entry;
        }
        else {
            // otherwise need to swap last entry to position of deleted
            if (block_to_modify_fat_index == curr_fat_index) {
                curr_block_ar[to_delete_block_ar_index] = curr_dir_entry;
                curr_block_ar[ar_counter - 1] = deleted_dir_entry;
            }
            else {
                fseek(disk, find_offset(block_to_modify_fat_index), SEEK_SET);
                fread(block_to_modify_ar, BLOCK_SIZE, 1, disk);
                block_to_modify_ar[to_delete_block_ar_index] = curr_dir_entry;
                curr_block_ar[ar_counter - 1] = deleted_dir_entry;
                fseek(disk, find_offset(block_to_modify_fat_index), SEEK_SET);
                fwrite(block_to_modify_ar, BLOCK_SIZE, 1, disk);
            }
        }
        // write last block back
        fseek(disk, find_offset(curr_fat_index), SEEK_SET);
        fwrite(curr_block_ar, BLOCK_SIZE, 1, disk);
    }

    // case 2: block containing file to delete contains parent dir header, but parent dir is multiblock
    else if ((curr_fat_index != parent_dir->first_FAT_idx) && (block_to_modify_fat_index == parent_dir->first_FAT_idx)) {
        // need to swap last entry to position of deleted
        parent_dir->data_in_first_block[to_delete_block_ar_index] = curr_dir_entry;
        curr_block_ar[ar_counter - 1] = deleted_dir_entry;
        // write last block back
        fseek(disk, find_offset(curr_fat_index), SEEK_SET);
        fwrite(curr_block_ar, BLOCK_SIZE, 1, disk);
    }

    // case 3: parent dir is singleblock
    else if (curr_fat_index == parent_dir->first_FAT_idx) {
        // check if file entry to delete is at end of dir entries
        if (to_delete_file_index == (files_in_parent_dir - 1)) {
            // if so just delete the last entry in the last block
            parent_dir->data_in_first_block[to_delete_block_ar_index] = deleted_dir_entry;
        }
        else {
            // otherwise need to swap last entry to position of deleted
            parent_dir->data_in_first_block[to_delete_block_ar_index] = curr_dir_entry;
            parent_dir->data_in_first_block[ar_counter - 1] = deleted_dir_entry;
        }
    }

    // unhandled case
    else {
        return EXIT_FAILURE;
    }

    // fix parent dir header
    parent_dir->size = parent_dir->size - DIR_ENTRY_BYTES;
    fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
    fwrite(parent_dir, BLOCK_SIZE, 1, disk);

    if(parent_dir->first_FAT_idx != 0){ // not root directory {
    file_handle dum; // grand parent 
    dum.parent_FAT_idx = parent_dir->data_in_first_block[1].first_FAT_idx;
    dum.first_FAT_idx = parent_dir->first_FAT_idx;
    set_file_size(&dum,parent_dir->size);
    }
    // finally fix fat table and add deleted file blocks to free list

    fat_entry empty_fat_entry;
    empty_fat_entry.next = UNUSED_BLOCK;
    fat_entry terminating_fat_entry;
    terminating_fat_entry.next = -1;

    // first handle case where parent dir is using one less block
    fat_entry fat_entry_step = fat_table[parent_dir->first_FAT_idx];
    int last_fat_step_index = parent_dir->first_FAT_idx;
    int double_last_fat_step_index = -1;
    if ( (((parent_dir->size) - FILE_HEADER_BYTES) % (BLOCK_SIZE / DIR_ENTRY_BYTES)) == (FILE_AFTER_HEADER_BYTES / DIR_ENTRY_BYTES) ) {
        // look through fat table to find last block of parent dir
        while (fat_entry_step.next != -1) {
            double_last_fat_step_index = last_fat_step_index;
            last_fat_step_index = fat_entry_step.next;
            fat_entry_step = fat_table[fat_entry_step.next];
        }
        // set it to be unused and then update free list
        fat_table[last_fat_step_index] = empty_fat_entry;
        fat_table[double_last_fat_step_index] = terminating_fat_entry;

        aux_insert_into_freelist(last_fat_step_index);

        /*
        if (global_superblock->free_block == NONE_FREE) {
            free_datablock new_first_free_db;
            new_first_free_db.next = -1;
            global_superblock->free_block = new_first_free_db;
            fseek(disk, find_offset(last_fat_step_index), SEEK_SET);
            fwrite(new_first_free_db, BLOCK_SIZE, 1, disk);
            fseek(disk, 0, SEEK_SET);
            fwrite(global_superblock, BLOCK_SIZE, 1, disk);
        }
        else {
            free_datablock new_free_db;
            struct free_datablock head_of_free_list;
            fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
            fread(&head_of_free_list, BLOCK_SIZE, 1, disk);
            new_free_db.next = head_of_free_list.next;
            //fseek(disk, find_offset(curr_fat_index), SEEK_SET);
            fseek(disk, find_offset(last_fat_step_index), SEEK_SET);
            fwrite(&new_free_db, BLOCK_SIZE, 1, disk);
            head_of_free_list.next = curr_fat_index;
            //fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
            fseek(disk, find_offset(last_fat_step_index), SEEK_SET);
            fwrite(&head_of_free_list, BLOCK_SIZE, 1, disk);
            }
        }
        */
    }

    // now free all blocks of deleted file

    int blocks_to_free = (to_delete_dir_entry.size / BLOCK_SIZE) + 1;
    int fat_index_to_free = to_delete_dir_entry.first_FAT_idx;
    int next_fat_index;
    fat_entry fat_entry_to_free;
    for (int i = 0; i < blocks_to_free; i++) {
        aux_insert_into_freelist(fat_index_to_free);
        fat_entry_to_free = fat_table[fat_index_to_free];
        next_fat_index = fat_entry_to_free.next;
        fat_table[fat_index_to_free] = empty_fat_entry;
        fat_index_to_free = next_fat_index;
    }

    //struct free_datablock overwrite_db;
    //struct free_datablock head_of_free_list;
    //struct blk_to_free_fat_index = to_delete_dir_entry.first_FAT_idx;
    //fseek(disk, find_offset(blk_to_free_fat_index))

    // write the up to date fat table
    fseek(disk, SUPERBLOCK_BYTES, SEEK_SET);
    fwrite(&fat_table, FATTABLE_BYTES, 1, disk);

    // free malloced stuff
    free(parent_dir);
    free(tokens);
    free(file_e);
    free(name);
    return EXIT_SUCCESS;
}

int auxmygetchar ( void ) 
{
  int ch;
  struct termios oldt, newt;
  
  tcgetattr ( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );
  
  return ch;
}

int f_minimore(const char * pathname) {
    //char absolute_path[1000];
    //absPathFromDir(relative_path, absolute_path);

    //tokenizing the pathname 
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    char *name = malloc(sizeof(char)*9);
    strcpy(name,tokens[token_length-1]);
    if ((strlen(name) + 1) > NAME_BYTES) {
        //printf("Name too long\n");
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
    for(int i = 0; i < token_length; i++){
        status = find_file_from_directory(file_e,fat_e,tokens[i],NULL);

        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            //printf("FILE NOT FOUND\n");
            return EXIT_FAILURE;
        }
    }//file_e should be the file
    

    int blocks_in_file = ((file_e->size) / BLOCK_SIZE) + 1;
    char curr_data[BLOCK_SIZE + 1];
    fat_entry curr_fat_entry = fat_table[file_e->first_FAT_idx];
    strncpy(curr_data, file_e->data_in_first_block, FILE_AFTER_HEADER_BYTES + 1);
    for (int i = 0; i < blocks_in_file; i++) {
        printf("%s\n", curr_data);
        auxmygetchar();
        if ( !((i + 1) == blocks_in_file) ) {
            fseek(disk, find_offset(curr_fat_entry.next), SEEK_SET);
            fread(curr_data, BLOCK_SIZE, 1, disk);
            curr_data[BLOCK_SIZE] = '\0';
            curr_fat_entry = fat_table[curr_fat_entry.next];
        }
    }

    free(tokens);
    free(file_e);
    free(name);
    return 0;
}

dir_handle *f_opendir(const char *pathname){
    file_header *file_e = malloc(sizeof(file_header));
    long cur_block = 0;

    //tokenizing the pathname
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    
    //***check if open files already contains the same file, then don't open

    //seeking the root directory fat entry
    fat_entry *fat_e = &fat_table[0];

    //seeking the data block for root
    fseek(disk,find_offset(0),SEEK_SET);
    fread(file_e,sizeof(*file_e),1,disk);

    int status = 0;
    int parent_FAT_idx = 0;
    //finding file from directory repeatedly
    for(int i = 0; i < token_length; i++){
        status = find_file_from_directory(file_e,fat_e,tokens[i],&parent_FAT_idx);
        
        //error checking
        if(status == EXIT_FAILURE){
            for(int i = 0; i < token_length; i++){
                free(tokens[i]);
            }
            free(tokens);
            free(file_e);
            //printf("FILE NOT FOUND\n");
            return NULL;
        }
        //update fat_entry
        fat_e = &fat_table[file_e->first_FAT_idx];
    }
    
    //error checking if the file is not a directory
    if(!file_e->is_directory){
        f_error = E_NOT_DIR;
        //printf("Opendir(): trying to open a non dir file, exiting...\n");
        return NULL;
    }
    //making a new file handle
    dir_handle *to_return = malloc(sizeof(dir_handle));
    to_return->r_index = 0; 
    to_return->cur_entry = malloc(sizeof(dir_entry));
    to_return->is_dir = 1;
    to_return->parent_FAT_idx = parent_FAT_idx;
    strcpy(to_return->name,file_e->name);
    to_return->size = file_e->size;
    to_return->first_FAT_idx = file_e->first_FAT_idx;

    //putting the file handle into the open_files array
    int status1 = 0;
    for(int i = 0; i < NUM_OPEN_FILES; i++){
        if(!open_files[i]){
            open_files[i] = (file_handle *)to_return;
            status1 = 1;
            break;
        }
    }
    if(!status1){
        //printf("Max number of open files reached\n");
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

dir_entry * f_readdir(dir_handle *directory){
    //returns the next dir_entry in the directory, updates the pointer to the current file // working for single block
    if (directory->is_dir != 1 || directory->size-FILE_HEADER_BYTES <= directory->r_index * sizeof(dir_entry)){ //we read to the end of dir_handle previously
        // dir_entry fail;
        // fail.first_FAT_idx = 0;
        return NULL;
    }
    if (directory->r_index < 15){ //data in first block
        dir_header * first_dir_block = malloc(sizeof(dir_header));
        fseek(disk,find_offset(directory->first_FAT_idx),SEEK_SET);
        fread(first_dir_block,sizeof(dir_header),1,disk);
        *(directory->cur_entry) = first_dir_block->data_in_first_block[directory->r_index++];
        free(first_dir_block);
        return directory->cur_entry;
    }
    else{ // data in any other block
        int page_num = (sizeof(dir_entry) * (directory->r_index + 1)) / BLOCK_SIZE; // page we need to seek to/load in; +1 for header in first block *assumes dir_entry and header are the same size
        printf("block: %d", page_num);
        int cur_fat = directory->first_FAT_idx;
        for (int i = 0; i < page_num; i++){
            cur_fat = fat_table[cur_fat].next;
        }
        dir_entry * dir_entries = malloc(BLOCK_SIZE);
        fseek(disk,find_offset(cur_fat),SEEK_SET);
        fread(dir_entries, BLOCK_SIZE,1,disk);
        *(directory->cur_entry) = dir_entries[(directory->r_index+1)  % (BLOCK_SIZE/sizeof(dir_entry))];
        directory->r_index++;
        free(dir_entries);
        return directory->cur_entry;
    }
}

int f_closedir(dir_handle *stream){
    //close an open directory file, cleans up memory in the open files list
    for(int i = 0; i < NUM_OPEN_FILES; i++){
        if(open_files[i] && open_files[i]->first_FAT_idx == stream->first_FAT_idx){
            //found the file
            open_files[i] = NULL;
        }
    }   
    free(stream->cur_entry); // malloced dir_entry ptr
    free(stream);
}

int f_rmdir(const char *pathname){
    //delete a directory, removes entire contents and the contents of all subdirectorys from the filesystem
}

// int main(){
//     f_init(101,"fresh_disk");

//     //***testing f_write
//     // file_handle *temp = f_open("beemovie",READ_WRITE);
//     // char *a = "TEST!!!!@@@@";
//     // f_seek(temp,0,SEEK_SET);
//     // for(int i = 0; i < 100; i ++){
//     //     f_write(a,12,1,temp);
//     // }
    
//     // char *buffer = malloc(1200);
//     // f_seek(temp,480,SEEK_SET);
//     // f_seek(temp,0,SEEK_SET);
//     // f_read(buffer,1200,1,temp);
//     // // f_read(buffer,800,1,temp);
//     // free(buffer);
//     // f_close(temp);

//     //***testin f_mkdir
//     f_mkdir("/next","e");
//     f_mkdir("/jeig/hi","e"); //shouldn't work because jeig/ is not in root
//     f_mkdir("/eigob;","e"); //shouldn't work because contains a bad character ';'
//     f_mkdir("/next","e"); //shouldn't work because next already exists in root
//     f_mkdir("/hi","e");
//     f_mkdir("hi2","e");
//     f_mkdir("hi3","e");
//     f_mkdir("hi4","e");
//     f_mkdir("hi5","e");
//     f_mkdir("hi6","e");
//     f_mkdir("hi7","e");
//     f_mkdir("hi8","e");
//     f_mkdir("hi9","e");
//     f_mkdir("hi10","e");
//     f_mkdir("hi11","e");
//     f_mkdir("hi12","e");
//     f_mkdir("hi13","e");
//     f_mkdir("hi14","e");
//     f_mkdir("/hiiii","e");
//     f_mkdir("/hi/wow","e");
//     file_handle *test = f_open("/hiiii/",1);
//     file_handle *test1 = f_open("/hi/wow/",1);
//     dir_entry temp1[16];
//     fseek(disk,find_offset(16),SEEK_SET);
//     fread(&temp1, BLOCK_SIZE, 1, disk);
//     f_close(test);
//     f_close(test1);

//     //***test f_mkfile
//     // f_mkdir("/next","e");
//     // f_mkfile("/test.txt","e");
//     // f_mkfile("/next/test.txt","e");
//     // file_handle *test = f_open("/test.txt",READ_WRITE);
//     // //file_handle *test1 = f_open("/next/test.txt",1);
//     // char chars[1000];
//     // for(int i = 0; i < 1000; i ++){
//     //     chars[i] = '!';
//     //     if(i == 999){
//     //         chars[i] = '\0';
//     //     }
//     // }
//     // f_write(chars,1000,1,test);
//     // file_header temp1;
//     // fseek(disk,find_offset(3),SEEK_SET);
//     // fread(&temp1, BLOCK_SIZE, 1, disk);
//     // char *answer = malloc(1000);
//     // f_read(answer,1000,1,test);

//     // free(answer);
//     // f_close(test);


//     // dir_entry *a = malloc(32);
//     // temp->cur_rindex = 32;
//     // f_read(a,32,1,temp);
//     // file_handle* temp = f_open("/",READ_ONLY);
//     // f_seek(test,2,SEEK_SET);
//     // f_mkdir("/next/test","e");
//     f_terminate();
//     // int i = (1008+16) / 512;
//     // printf("%d\n",i);
// }
