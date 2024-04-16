#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "List.h"
#include "node.h"
#include "file_system.h"
#include "disk.h"


#define READONLY 1
#define WRITEONLY 2
#define READWRITE 3
#define APPEND 4

#define NAMEBYTES 8
#define BLOCKSIZE 512


extern List *open_files;
extern FILE *disk;
#endif