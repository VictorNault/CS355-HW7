#ifndef BUILT_IN_CMDS_H
#define BUILT_IN_CMDS_H
#include "stdio.h"
#include "common.h"
#include "node.h"
#include "../CS355-HW7/file_system.h"

int cat(char ** files, int numFiles, char * dest,int mode);
int ls(char ** command, int length, char * dest, int mode);
void bg(int pid);
void fg(int pid);
int myKill(int pid, int isSIGKILL);
void printJobs(List * processes);
void testing(char * infile);
#endif