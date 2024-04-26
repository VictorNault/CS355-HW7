int f_seek(file_handle *stream, long offset, int position){
    

    if(position == SEEK_SET){
        stream->cur_rindex = offset;
        stream->cur_windex = offset;
        return EXIT_SUCCESS;
    } 
    else if(position == SEEK_CUR){   
        stream->cur_rindex += offset;
        stream->cur_windex += offset;
        return EXIT_SUCCESS;
    } else if(position == SEEK_END){
        stream->cur_rindex = (TOTAL_BYTES - offset);
        stream->cur_windex = (TOTAL_BYTES - offset);
        return EXIT_SUCCESS;
    } else {
        return -1;
    }
    //move pointers to a specified position in a file
}
