#ifndef BUILT_IN_CMDS_H
#define BUILT_IN_CMDS_H
#include "stdio.h"
#include "common.h"
#include "node.h"
int cat(char ** files, int numFiles, FILE * dest);
void bg(int pid);
void fg(int pid);
int myKill(int pid, int isSIGKILL);
void printJobs(List * processes);
#endif