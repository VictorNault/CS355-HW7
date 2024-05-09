#ifndef COMMON2_H
#define COMMON2_H
#define _XOPEN_SOURCE 700
#define TRUE 1
#define FALSE 0
#define TOSTRING_SIZE 1024


#define READ_ONLY 1
#define WRIT_EONLY 2
#define READ_WRITE 3
#define APPEND 4


#define NUM_OPEN_FILES 50
#define BLOCK_SIZE 512
#define NUM_FAT_ENTRIES 1024

//errors
#define E_FILE_NOT_FOUND 2
#define E_OUT_OF_BOUNDS 3

//disk
#define NAME_BYTES 9
#define TOTAL_BYTES 1048576
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define SUPERBLOCK_PADDING 492
#define FILE_AFTER_HEADER_BYTES 480
#define TABLE_OFFSET 1
#define TABLE_BLOCKS 16
#define FIXED_FREEBLOCK 2
#define UNUSED_BLOCK -2
#define TRUE 1
#define FALSE 0

#define FREE_DATABLOCK_EXTRA_BYTES 508
#define PROT_BYTES 11
#define NONE_FREE -1
#define FILE_HEADER_BYTES 32
#define DIR_ENTRY_BYTES 32

#include <pthread.h>

#include <regex.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>  
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>       
#include <dirent.h>
#include <math.h>
#include <sys/types.h>
#include "List.h"
#include "List_Extras.h"
#include "node.h"
#include "../CS355-HW7/file_system.h"
#include "built_in_commands.h"
#include "commands.h"
#include "Process_Props.h"
#include "sighandlers.h"
#include "string_extras.h"
#include "parser.h"
#include "formatDISK.h"
extern List * processes;
extern pthread_mutex_t mutex; //defining the mutex
extern pid_t shellPid;
extern struct termios shellTermios;
extern char global_workingPath[1028]; // current path 


#endif