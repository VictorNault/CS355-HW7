#ifndef COMMON_H
#define COMMON_H


#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4


#define NUM_OPEN_FILES 50
#define NAME_BYTES 8
#define BLOCK_SIZE 512
#define NUM_FAT_ENTRIES 1024

//errors
#define FILE_NOT_FOUND 2


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "disk.h"
#include "List.h"
#include "node.h"
#include "file_system.h"




extern file_entry open_files[];
extern FILE *disk;
#endif