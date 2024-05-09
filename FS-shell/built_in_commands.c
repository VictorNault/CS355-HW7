#include "common.h"
#include "../CS355-HW7/file_system.h"

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
char global_workingPath[1028];

// char * checkPerms(char * userID, file_handle){
// }
 // probably not right need to add / somewhere 
char * convertToAbsPath(char * relativePath, int * isMalloced){
    if (relativePath[0] == '/'){ //recieved absolute path 
        isMalloced = FALSE;
        return relativePath;
    }

    // char ** currentDirToks = tokenize(currentPath,&cur_dir_len,"/");
    char * abs_path = malloc(sizeof(char) + strlen(relativePath) + strlen(global_workingPath) + 1 + 1000);
    memcpy(abs_path,global_workingPath,strlen(global_workingPath));
    memcpy(abs_path+strlen(global_workingPath),relativePath, strlen(relativePath)+1);
    *isMalloced = TRUE;
    return abs_path;
}
//confirm path actually exisits first consider passing dir_handle instead of path name.
int absPathFromDir(char * path, char * output){
    dir_handle * cur_dir = f_opendir(path);
    // dir_entry * cur_entry = f_readdir(cur_dir);
    if (cur_dir == NULL){

        return EXIT_FAILURE;
    }
    cur_dir->r_index = 1;

    if (strcmp(cur_dir->name,"root") == 0){
        strcat(output,"/");
        return 0;
    } 
    else{
    strcat(path, "/..");
    // strcat(path, cur_entry->name);
    int status = absPathFromDir(path, output);
    if (status == 0){
    strcat(output,cur_dir->name);   
    strcat(output,"/");
    }
    f_closedir(cur_dir);
    return EXIT_SUCCESS;
    }
}
// open dir test 
void testing(char * path){
    // fseek(disk,17 * BLOCK_SIZE,SEEK_CUR);
    // dir_header * buf = malloc(sizeof(dir_header));
    // fread(buf, 512,1, disk);



    // file_handle * myfile = f_open("beemovie",READ_ONLY);
    // char * buf = malloc(sizeof(char) * 1024);
    // f_read(buf,512,1,myfile );
    //     printf("buf: %s\n", buf);

    // f_read(buf,512,1,myfile );
    // printf("buf: %s\n", buf);
    char * output = malloc(1000);
    char * path2 = malloc(1000);
    strcpy(path2, "/dir1/dir2/.");
    absPathFromDir(path2,output);
    printf("%s",output);

}
// cat displays the content of one or more files to the output.

int cat(char ** command,int numFiles,char * dest, int mode){
    char * buf = malloc(sizeof(char)*1);
    file_handle * outFile = NULL;
    if (dest != NULL){
        int isMalloced;
        char * outputPath = convertToAbsPath(dest, &isMalloced);
        outFile = f_open(outputPath,mode);
        free(outputPath);
    }
    if (numFiles == 1){ // just cat, read from stdin
        int status = 1;
        while (status != 0)
        {
           status = fread(buf,sizeof(char),1,stdin);
           printf("%c",*buf);
        } 
        free(buf);
        return 0;
    }
    // starting at 1 to cut out cat
    printf("size of command: %d",numFiles);
    for (int i = 1; i < numFiles; i++){
        file_handle * curFile = f_open(command[i],READ_ONLY);
        if (curFile == NULL){
            printf("\033[0;31mError:\001\e[0m\002 No File or Directory\n");
            continue;
        }
        int status = f_read(buf, sizeof(char),1,curFile);
        // printf("%c",*buf);
        while (status != 0){
            // printf("dest is null %d\n", dest == NULL);
            // printf("%c",*buf);
            if (dest == NULL){
                printf("%c",*buf);
            }
            else{
                f_write(buf,sizeof(char),1,outFile);    
            }
            status = f_read(buf, sizeof(char),1,curFile);
        }
        // fclose(curFile);
    }
    free(buf); 
    return EXIT_SUCCESS;    
}


//Will make a call to opendir, which returns a dir entry, which has list of file entry
int ls(char ** command, int length, char * dest, int mode){
    int l_flag = 0;
    int skip_idx = -1;
    file_handle * outFile = NULL;
    //checking for -l flag
    for(int i = 0; i < length;i++){
        if (strcmp(command[i],"-l") == 0){
            l_flag = 1; // setting l flag
            skip_idx = i;
        }
    }

    if (length == 1 || (length == 2 && l_flag == 1)){ // just ls
    dir_handle * directory = f_opendir(global_workingPath);
    directory->r_index=2;
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
                if (outFile == NULL) printf("%s\n", curdir->name); //stdout
                else f_write(curdir->name,strlen(curdir->name)+1,1,outFile);
            }
            else{                    
                char * permStr = arrayToPermStr(curdir->protection, curdir->is_directory);
                if (outFile == NULL) printf("%s %d %d %s\n",permStr,curdir->uid,curdir->first_FAT_idx,curdir->name);
                else{
                    char str[256];
                    sprintf(str,"%s %d %d %s\n",permStr,curdir->uid,curdir->first_FAT_idx,curdir->name);
                    f_write(str,sizeof(str),1,outFile);
                }
                free(permStr);
            }
            curdir = f_readdir(directory);
    }
    return 0;
    }


    for (int i = 1; i < length;i++ )
    {
        if (i == skip_idx) continue;
        int isMalloced;
        char * absPath = convertToAbsPath(command[i], &isMalloced);
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
    //f_closedir(directory);
}

//chmod changes the permissions mode of a file. Support absolute mode and symbolic mode.
// will need to parse symbols and convert to octal.
//u = owner, g = group, o = others, a = all, =, +, -, r,w,x. e.g u=rwx gives owner read, write, execute
void  chmod(file_handle * file, char * permissions){ //will probably fwrite to specific bitsin the file header
    int isMalloced;
    char * absPath = convertToAbsPath(file,&isMalloced);
    file_handle * file_to_update = f_open(absPath, READ_ONLY);
    if (file_to_update == NULL){
        printf("\033[0;31mError:\001\e[0m\002 Invalid file or directory\n");
    }
    u_int8_t * newperms = chmodParsing(permissions, file->is_dir, NULL); //file->permssions);
    if (newperms == NULL){
        return;
    }
    update_protection(file->parent_FAT_idx, file->name,newperms);
    free(absPath);
    f_close(file);
    // need something to write changes to disk

};

//mkdir creates a directory
void mkdirFS(char ** command, int commandLength){
    for (int i = 1; i < commandLength; i++){
        int isMalloced;
        char * absPath = convertToAbsPath(command[i],&isMalloced);
        f_mkdir(command[i],0);
        if (isMalloced == TRUE) free(absPath);
    }
}// call make directory after finding the current path

//rmdir removes a directory
void rmdirFS(char * directoryName){} // call remove directory after finding current path // recursive :) 

int cd(char ** command, int length){ //changiing working path, needs to parse for .., 
    if (length != 2){
        printf("\033[0;31mError:\001\e[0m\002 Invalid Parameters\n");
        return; 
    }
    int isMalloced;
    char * absPath = convertToAbsPath(command[1], &isMalloced);
    if (isMalloced == FALSE){
        char * temp = absPath;
        char absPath[1028];
        strcpy(absPath,temp);
    }
    char pathcopy[1028];
    strcpy(pathcopy, global_workingPath);
    strcpy(global_workingPath, "");
    int status = absPathFromDir(absPath, global_workingPath);

    if (isMalloced == TRUE){
        free(absPath);
    }
    if (status == EXIT_FAILURE){ // wrtie the old working path back and error
        strcpy(global_workingPath,pathcopy);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
//prints working directory
void pwd(){
    printf("%s\n",global_workingPath);
}

// //more lists a file a screen at a time
// int more(char ** command, int length, char * dest, int mode){ // two cases one 
//     if (dest != NULL){
//        int status = cat(command, length, dest, mode);
//        return status;
//     }

//     char * buf = malloc(sizeof(char)*1);
//     file_handle * outFile = NULL;
//     if (dest != NULL){
//         char * outputPath = convertToAbsPath(dest);
//         outFile = f_open(outputPath,mode);
//         free(outputPath);
//     }
//     if (length == 1){ // just cat, read from stdin
//         int status = 1;
//         printf("need more :)");
//     }
//     // starting at 1 to cut out cat
//     printf("size of command: %d",numFiles);
//     for (int i = 1; i < numFiles; i++){
//         file_handle * curFile = f_open(command[i],READ_ONLY);
//         if (curFile == NULL){
//             printf("\033[0;31mError:\001\e[0m\002 No File or Directory\n");
//             continue;
//         }
//         int status = f_read(buf, sizeof(char),1,curFile);
//         // printf("%c",*buf);
//         while (status != 0){
//             // printf("dest is null %d\n", dest == NULL);
//             // printf("%c",*buf);
//             if (dest == NULL){
//                 printf("%c",*buf);
//             }
//             else{
//                 f_write(buf,sizeof(char),1,outFile);    
//             }
//             status = f_read(buf, sizeof(char),1,curFile);
//         }
//         // fclose(curFile);
//     }
//     free(buf); 
//     return EXIT_SUCCESS;    

// } //not sure how to approach this one

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