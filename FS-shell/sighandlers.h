#ifndef SIGHANDLERS_H
#define SIGHANDLERS_H
#include "common.h"
#include <signal.h>

void sigchldhandler(int signal, siginfo_t *info, void *ucontext);

void dummy(int signal, siginfo_t *info, void *ucontext);

#endif
