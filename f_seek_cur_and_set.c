
    if(position == SEEK_SET){
        stream->cur_rindex = offset;
        stream->cur_windex = offset;
        return EXIT_SUCCESS;
    } 
    else if(position == SEEK_CUR){   
        stream->cur_rindex += offset;
        stream->cur_windex += offset;
        return EXIT_SUCCESS;
    } 
