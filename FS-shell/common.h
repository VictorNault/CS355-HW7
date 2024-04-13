#ifndef COMMON_H
#define COMMON_H
#define _XOPEN_SOURCE 700
#define TRUE 1
#define FALSE 0
#define TOSTRING_SIZE 1024

#include <pthread.h>
#include "List.h"
#include "List_Extras.h"
#include "node.h"
#include "built_in_commands.h"
#include "commands.h"
#include "Process_Props.h"
#include "sighandlers.h"
#include "string_extras.h"
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
extern List * processes;
extern pthread_mutex_t mutex; //defining the mutex
extern pid_t shellPid;
extern struct termios shellTermios;


#endif