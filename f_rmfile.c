int f_rmfile(const char *pathname) {
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
    dir_entry curr_block_ar[BLOCK_BYTES / DIR_ENTRY_BYTES];
    for (int i = 0; i < possible_files_left_in_block; i++) {
        curr_block_ar[i] = parent_dir->data_in_first_block[i];
    }

    int to_delete_index = -1;
    dir_entry curr_dir_entry;
    int ar_counter = 0;
    int block_to_modify_fat_index = -1;
    fat_entry curr_fat_entry = fat_table[parent_dir->first_FAT_idx];
    int curr_fat_index = parent_dir->first_FAT_idx;
    for (int i = 0; i < files_in_parent_dir; i++) {
        if (possible_files_left_in_block == 0) {
            possible_files_left_in_block = BLOCK_BYTES / DIR_ENTRY_BYTES;
            ar_counter = 0;
            // get next block
            fseek(disk, find_offset(curr_fat_entry.next), SEEK_SET);
            fread(curr_block_ar, BLOCK_SIZE, 1, disk);
            curr_fat_index = curr_fat_entry.next;
            curr_fat_entry = fat_table[curr_fat_entry.next];
        }
        curr_dir_entry = curr_block_ar[ar_counter];
        if (strcmp(name, curr_dir_entry.name) == 0) {
            to_delete_index = i;
            block_to_modify_fat_index = curr_fat_index;
            if (curr_dir_entry.is_directory == TRUE) {
                // need "not file" error
                //errno = 
                return EXIT_FAILURE;
            }
        }
        --possible_files_left_in_block;
        ++ar_counter;
    }

    if (to_delete_index == -1) {
        errno = E_FILE_NOT_FOUND;
        return EXIT_FAILURE;
    }

}
