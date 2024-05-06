#include "common.h"
#include "CS355-HW7/file_system.h"

//Design for parsing >, >>, < 

//will need a file to translate relative path to absolute path (just strcat with current path, and ./ , or  )

// will add a new global called destination (default is STDOUT), which is set by parser when it reads one of the symbols above
// if ">" opens file with w+, if ">>" opens file with a+, < not sure what it does yet.
// need to think more about how this would work with multiple re-directs in one line

//getting paths, working directory as a global variable maybe of strings will only be updated by cd, and calls open dir repeaditly until we are at the working directory
// char ** tokenize(const char * stringToSplit, int * cmdLen, char* delimiters){ 
//     //counting the number of arguments passed by calling strtok twice (not the most efficient :()
//     char * stringToSplitCopy = malloc( sizeof(char) * (strlen(stringToSplit)+1));
//     strcpy(stringToSplitCopy, stringToSplit); 
//     char * token = strtok(stringToSplitCopy,delimiters);
//     *cmdLen = 0;
//     while (token != NULL){
//         (*cmdLen)++;
//         token = strtok(NULL,delimiters);
//     }

//     if (*cmdLen == 0){ //nothing was passed
//         return NULL;
//     }

//     char ** tokenList = (char **) malloc(sizeof(char *) * (*cmdLen));

//     strcpy(stringToSplitCopy, stringToSplit); 
//     token = strtok(stringToSplitCopy, delimiters);
//     tokenList[0] = (char *) malloc(sizeof(char)* (strlen(token)+1));
//     strcpy(tokenList[0],token);
//     for (int i = 1; i < *cmdLen; i++){
//         token = strtok(NULL, delimiters);
//         tokenList[i] = (char *) malloc(sizeof(char)* (strlen(token)+1));
//         strcpy(tokenList[i],token);
//     }

//     free(stringToSplitCopy);
//     return tokenList;
// }
char * global_workingPath;
char * convertToAbsPath(char * relativePath){
    if (relativePath[0] == '/'){ //recieved absolute path 
        return relativePath;
    }

    // char ** currentDirToks = tokenize(currentPath,&cur_dir_len,"/");
    char * abs_path = malloc(sizeof(char) + strlen(relativePath) + strlen(global_workingPath) + 1);
    memcpy(abs_path,global_workingPath,strlen(global_workingPath));
    memcpy(abs_path+strlen(global_workingPath),relativePath, strlen(relativePath)+1);
    return abs_path;
}

// open dir test 
void testing(char * path){
    // fseek(disk,17 * BLOCK_SIZE,SEEK_CUR);
    // dir_header * buf = malloc(sizeof(dir_header));
    // fread(buf, 512,1, disk);



    file_handle * myfile = f_open("beemovie",READ_ONLY);
    char * buf = malloc(sizeof(char) * 1024);
    f_read(buf,512,1,myfile );
        printf("buf: %s\n", buf);

    f_read(buf,512,1,myfile );
    printf("buf: %s\n", buf);
    
}
// cat displays the content of one or more files to the output.

int cat(char ** command,int numFiles,FILE * dest){
    char * buf = malloc(sizeof(char)*1024);
    if (dest == NULL) dest = stdout;
    if (numFiles == 1){ // just cat, read from stdin
        int status = 1;
        while (status != 0)
        {
           status = fread(buf,sizeof(char),1,stdin);
           fwrite(buf,sizeof(char),1,dest);
        } 
        free(buf);
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
        int status = fread(buf, sizeof(char),1,curFile);
        // printf("%c",*buf);
        while (status!= 0){
            // printf("%c",*buf);
            fwrite(buf,sizeof(char),1,dest);    
            status = fread(buf, sizeof(char),1,curFile);
        }
        fclose(curFile);
    }
    free(buf); 
    return EXIT_SUCCESS;    
}


//Will make a call to opendir, which returns a dir entry, which has list of file entry
int ls(char ** command, int length){
    int l_flag = 0;
    int skip_idx = -1;

    //checking for -l flag
    for(int i = 0; i < length;i++){
        if (strcmp(command[i],"-l") == 0){
            l_flag = 1; // setting l flag
            skip_idx = i;
        }
    }

    if (length == 1 || (length == 2 && l_flag == 1)){ // just ls
    dir_handle * directory = f_opendir(global_workingPath);
    if (!directory) {  // failed to open file
        // todo: handle case where it's not a directory but valid file (just print fstat :) )
        // int status = fstat(global_workingPath);
        // if (status == -1){

        // }
        // else{

        // }
        return 0;
    }
    dir_entry * curdir = f_readdir(directory);
    while(curdir){
            if (l_flag != 1){
            printf("%s\n", curdir->name);
            }
            else{
                printf("%d protection %d %d %s\n",curdir->is_directory,curdir->uid,curdir->first_FAT_idx,curdir->name);
            }
            curdir = f_readdir(directory);
    }
    return 0;
    }


    for (int i = 1; i < length;i++ )
    {
        if (i == skip_idx) continue;
        char * absPath = convertToAbsPath(command[i]);
        dir_handle * directory = f_opendir(absPath);
        if (!directory) {  // failed to open file
            printf("\033[0;31mError:\001\e[0m\002 File not found\n");
            return -1;
        }
        dir_entry * curdir = f_readdir(directory);
        while(curdir){
            if (l_flag != 1){
            printf("%s\n", curdir->name);
            }
            else{
                printf("%d protection %d %d %s\n",curdir->is_directory,curdir->uid,curdir->first_FAT_idx,curdir->name);
            }
            curdir = f_readdir(directory);
        }
    }
}

//chmod changes the permissions mode of a file. Support absolute mode and symbolic mode.
// will need to parse symbols and convert to octal.
//u = owner, g = group, o = others, a = all, =, +, -, r,w,x. e.g u=rwx gives owner read, write, execute
void  chmod(char * file){}; //will probably fwrite to specific bitsin the file header

//mkdir creates a directory
void mkdirFS(char ** command, int commandLength){
    for (int i = 1; i < commandLength; i++){
        char * absPath = convertToAbsPath(command[i]);
        f_mkdir(command[i],0);
        free(absPath);
    }
}// call make directory after finding the current path

//rmdir removes a directory
void rmdirFS(char * directoryName){} // call remove directory after finding current path // recursive :) 

void cd(char * path){ //changiing working path, needs to parse for .., .
    // start by tokenizing the path 
    // if ()

}
//prints working directory
void pwd(){
    printf("%s\n","wokringPath");
}

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