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
