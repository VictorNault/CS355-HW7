#include "commands.h"

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
    

    //check in the linked list to see if the process is suspended
    //int is_suspended = 
    // if(is_suspended){
    //     kill(pid,SIGCONT);
    //     tcsetpgrp(1,pid); //set a process to run in foreground

    // }
    // else{
    //     tcsetpgrp(1,pid);
    // }
    //     //entering CR
    //      ***update the is_suspended flag in LL
    //      ***update the is_foreground flag in LL
    //      //exiting CR

    //waitpid(pid,NULL,0);
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