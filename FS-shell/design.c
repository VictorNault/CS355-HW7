#define _XOPEN_SOURCE 700
#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <string.h>
#include <pthread.h>

//including the linked list data structure
#include "List.h"

pid_t pid;

//define a global string delimeter
//define a global linked list to store processes

int bg(int pid){
    //make a process go to the background
}

int fg(int pid){
    //make a process go to the foreground
}

void jobs(){
    //print processes, oldest in the front, youngest in the back
}

void kill(int pid, int sig){
    //send a SIGTERM or SIGKILL to a process
}

//in the sigchldhandler, we will use mutexes to protect the critical region, which is
//whenever the linkedlist containing process info is modified
void sigchldhandler(siginfo_t *siginfo, void *context){
    //if child is exited, or dumped
        //Entering CR
        //delete the node from the linked list
        //Exiting CR

    //if child is stopped (suspended), update the flag
        //Entering CR
        //update the flag
        //Exiting CR
    
    //if child is resumed (continued), update the flag
        //Entering CR
        //update the flag
        //Exiting CR
}

int main(){
    //define sigaction to mask all signals (sa_mask)

    //set up sighandler for SIGINT 

    //define a linked list buffer for potential multiple commands in case of semicolon

    while(TRUE){
        //define a flag for foreground or background

        //if there isn't a command in the buffer linked list

            //get the command string using readline

            //if the command string is null (Ctrl+D), exit the shell

            //parse the command string, store tokens in an array


        //fork off a child of a unique process group

        //if background, use tcsetpgrp to make the child not read from terminal

        //child 
        if(pid == 0){
            //entering CR
            //add the child's process to the linkedlist
            //exiting CR
            //exec
            //if fails, exit
        }
        //parent
        else{
            //

            //wait for child only if child is in foreground
        }
    }
}

