#include "common.h"

//Design for parsing >, >>, < 

//will need a file to translate relative path to absolute path (just strcat with current path, and ./ , or  )

// will add a new global called destination (default is STDOUT), which is set by parser when it reads one of the symbols above
// if ">" opens file with w+, if ">>" opens file with a+, < not sure what it does yet.
// need to think more about how this would work with multiple re-directs in one line


//getting paths, working directory as a global variable maybe of strings will only be updated by cd, and calls open dir repeaditly until we are at the working directory




// cat displays the content of one or more files to the output.
int cat(char ** command,int numFiles,FILE * dest){
    if (dest == NULL) dest = stdout;
    if (numFiles == 1){ // just cat, read from stdin
        char buf[1];
        int status = 1;
        while (status != 0)
        {
           status = fread(buf,sizeof(buf),1,stdin);
           fwrite(buf,sizeof(buf),1,dest);
        } 
        return 0;
    }
    // starting at 1 to cut out cat
    printf("size of command: %d",numFiles);
    for (int i = 1; i < numFiles; i++){
        FILE * curFile = fopen(command[i],"r");
        if (curFile == NULL){
            printf("\033[0;31mError:\001\e[0m\002 No File or Directory\n");
            continue;
        }
        char buf[1];
        int status = fread(buf, sizeof(char),1,curFile);
        // printf("%c",*buf);
        while (status!= 0){
            // printf("%c",*buf);
            fwrite(buf,sizeof(char),1,dest);    
            status = fread(buf, sizeof(char),1,curFile);
        }
        fclose(curFile);
    }
    return EXIT_SUCCESS;    
}

//Will make a call to opendir, which returns a dir entry, which has list of file entry
void ls(char ** path){}


//chmod changes the permissions mode of a file. Support absolute mode and symbolic mode.
// will need to parse symbols and convert to octal.
//u = owner, g = group, o = others, a = all, =, +, -, r,w,x. e.g u=rwx gives owner read, write, execute
void chmod(char * file){}; //will probably fwrite to specific bitsin the file header

//mkdir creates a directory
void mkdirFS(char * directoryName){}// call make directory after finding the current path

//rmdir removes a directory
void rmdirFS(char * directoryName){} // call remove directory after finding current path

void cd(char * path){} //changiing working path, needs to parse for .., .

//prints working directory
void pwd(){}

//more lists a file a screen at a time
void more(){} //not sure how to approach this one

//rm deletes a file
void rm(char * path){} // removes specific file


//might be job id actually
void bg(int pid){
    
    //check in the linked list to see if the process is suspended

    // also don't think we need to update is_suspended flag here since child will signal
    Process_Props * process;
    if(pid == -1){ // most recent

        pthread_mutex_lock(&mutex);
        if (empty(processes)){ // note tail handling might be off, when testing I originally had if tail = null which gave seg fault should check List_extras.v
            pthread_mutex_unlock(&mutex);
            return;
        }
        process = processes->tail->data;
        pthread_mutex_unlock(&mutex);
    }else{
        pthread_mutex_lock(&mutex);
        process = get_by_jobid(processes,pid);
        pthread_mutex_unlock(&mutex);
    }

    if(!process){
        printf("%d: job not found\n",pid);
        return;
    }

    if(process->is_suspended){
        kill(-1 * process->pid,SIGCONT);
        printf("[%d]    %s\n",process->job_id,process->starting_command);
    }else{
        printf("bg: job %d already in background\n",process->job_id);
    }

    //int is_suspended = 
    // if(is_suspended){
    //     kill(pid,SIGCONT);
    //     //entering CR
    //      ***update the is_suspended flag in LL
    //      //exiting CR
    // }
    //else don't do anything because process is already running in background
}

void fg(int pid){
    Process_Props * process;
    if(pid == -1){ // most recent
        pthread_mutex_lock(&mutex);
        if (empty(processes)){ // note tail handling might be off, when testing I originally had if tail = null which gave seg fault should check List_extras.v
            pthread_mutex_unlock(&mutex);
            return;
        }
        process = processes->tail->data;
        pthread_mutex_unlock(&mutex);
    }else{
        pthread_mutex_lock(&mutex);
        process = get_by_jobid(processes,pid);
        pthread_mutex_unlock(&mutex);
    }
    
    if(!process){
        printf("%d: process not found\n",pid);
        return;
    }

    tcsetpgrp(STDIN_FILENO, process->pid);
    kill(-1 * process->pid, SIGCONT);
    


    //same here as comment above // process->is_suspended = FALSE;
    process->in_foreground = TRUE;
    
    if (process->hasTermios){
    tcsetattr(STDIN_FILENO,TCSADRAIN, &process->process_termios);
    }
    
    waitpid(process->pid,NULL,WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shellPid);
    tcsetattr(STDIN_FILENO,TCSADRAIN, &shellTermios);
}

int myKill(int jobid, int isSIGKILL){
    pthread_mutex_lock(&mutex);
    Process_Props * process = get_by_jobid(processes,jobid);
    pthread_mutex_unlock(&mutex);
    if(!process){
        printf("\033[0;31mError:\001\e[0m\002 Invalid job id\n");
        return 0;
    }
    int success;
    if(isSIGKILL){
        success = kill(process->pid,SIGKILL);
    }else{
        success = kill(process->pid,SIGTERM);
    }
    
    return success;
}

void printJobs(List * processes){
    struct node * temp = processes->head;
    int processCount = 1;
    print_processes(processes);
}