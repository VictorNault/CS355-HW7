int f_stat(file_handle *stream, file_header *stat_buffer){
stat_buffer->first_FAT_idx = stream->first_FAT_idx;
    for(int i = 0; i < sizeof(stream->name); i++){
        stat_buffer->name[i] = stream->name[i];
    }
    stat_buffer->size = stream->size;
    
    fseek(disk, find_offset(stream->first_FAT_idx), SEEK_SET);
    file_header* temp = malloc(sizeof(struct file_header));
    fread(temp, sizeof(struct file_header), 1, disk);
    stat_buffer->is_directory = temp->is_directory;
    fread(stat_buffer->data_in_first_block, 480, 1, disk);
    
    free(temp);
}
