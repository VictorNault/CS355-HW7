#include "sighandlers.h"
pthread_mutex_t mutex; //defining the mutex

void dummy(int signal, siginfo_t *info, void *ucontext) {
    if(info->si_code == CLD_EXITED || info->si_code == CLD_DUMPED || info->si_code == CLD_KILLED){ //si_code in the info struct contai>
        //Entering critical region
        waitpid(info->si_pid,NULL,WNOHANG);
        pthread_mutex_lock(&mutex);
        //***delete child from ll
        Process_Props *process = get_by_pid(processes,info->si_pid);
        delete_process(processes,process);
        pthread_mutex_unlock(&mutex);
    }
    else if(info->si_code == CLD_STOPPED){
        //Entering critical region
        pthread_mutex_lock(&mutex);
        Process_Props *process = get_by_pid(processes,info->si_pid);
        set_is_suspended(process,TRUE);
        if (get_in_foreground(process)){
            tcgetattr(STDIN_FILENO,&process->process_termios); //getting process's termios
            process->hasTermios = TRUE;
        }
        set_in_foreground(process,FALSE);
        pthread_mutex_unlock(&mutex);
    }
    else if(info->si_code == CLD_CONTINUED){
        //Entering critical region
        pthread_mutex_lock(&mutex);
        Process_Props *process = get_by_pid(processes,info->si_pid);
        set_is_suspended(process,FALSE);
        pthread_mutex_unlock(&mutex);
    }
}

void sigchldhandler(int signal, siginfo_t *info, void *ucontext){
         dummy(signal, info, ucontext);
}
