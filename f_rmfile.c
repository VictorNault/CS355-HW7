void aux_insert_into_freelist(int fat_index_of_to_insert) {
    if (global_superblock->free_block == NONE_FREE) {
        free_datablock new_first_free_db;
        new_first_free_db.next = -1;
        global_superblock->free_block = new_first_free_db;
        fseek(disk, find_offset(fat_index_of_to_insert), SEEK_SET);
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
        fseek(disk, find_offset(fat_index_of_to_insert), SEEK_SET);
        fwrite(&new_free_db, BLOCK_SIZE, 1, disk);
        head_of_free_list.next = fat_index_of_to_insert;
        fseek(disk, find_offset(global_superblock->free_block), SEEK_SET);
        fwrite(&head_of_free_list, BLOCK_SIZE, 1, disk);
        }
    }
}

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
    dir_entry to_delete_dir_entry;
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
            to_delete_file_index = i;
            to_delete_block_ar_index = ar_counter;
            to_delete_dir_entry = curr_dir_entry;
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

    // if file to delete is not found
    if (to_delete_index == -1) {
        errno = E_FILE_NOT_FOUND;
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
    dir_entry block_to_modify_ar[BLOCK_BYTES / DIR_ENTRY_BYTES];

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
           fat_index_of_to_insert     // write earlier block back
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
    parent_dir->size = parent_dir_size - DIR_ENTRY_BYTES;
    fseek(disk, find_offset(parent_dir->first_FAT_idx), SEEK_SET);
    fwrite(parent_dir, BLOCK_SIZE, 1, disk);

    // finally fix fat table and add deleted file blocks to free list

    fat_entry empty_fat_entry;
    empty_fat_entry.next = UNUSED_BLOCK;
    fat_entry terminating_fat_entry;
    terminating_fat_entry.next = -1;

    // first handle case where parent dir is using one less block
    fat_entry fat_entry_step = fat_table[parent_dir->first_FAT_idx];
    int last_fat_step_index = parent_dir->first_FAT_idx;
    int double_last_fat_step = -1;
    if ( (((parent_dir->size) - FILE_HEADER_BYTES) % (BLOCK_SIZE / DIR_ENTRY_BYTES)) == (FILE_AFTER_HEADER_BYTES / DIR_ENTRY_BYTES) ) {
        // look through fat table to find last block of parent dir
        while (fat_entry_step.next != -1) {
            double_last_fat_step = last_fat_step_index;
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
    return 0;
}
