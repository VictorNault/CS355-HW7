void f_rewind(file_handle *stream){
    stream->cur_rindex = 0;
    stream->cur_windex = 0;
    //move pointers to the start of the file
}
