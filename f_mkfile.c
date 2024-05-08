int f_mkfile(const char *pathname, char *mode) {
    // need to update for multiblock dir support
    //creates a new directory file in the specified location
    if (global_superblock->free_block == NONE_FREE) {
        printf("No free blocks, exiting f_mkdir\n");
        errno = E_NO_SPACE;
        return EXIT_FAILURE;
    }
    

    //tokenizing the pathname 
    int token_length = 0;
    char** tokens = tokenize(pathname,&token_length,"/");
    char *name = malloc(sizeof(char)*9);
    strcpy(name,tokens[token_length-1]);
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
    int new_file_block
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
    printf("Non_header_bytes: %d\nfiles_in_parent: %d\n",non_header_bytes_in_parent,files_in_parent);
    parent_dir->data_in_first_block[files_in_parent] = *new_dir_entry;
    parent_dir->size = parent_dir->size + DIR_ENTRY_BYTES;
    fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
    fwrite(parent_dir, BLOCK_SIZE, 1, disk);


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
