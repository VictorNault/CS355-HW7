#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include "List.h"
#include "node.h"
#include "file_system.h"
#include <sys/types.h>
#include <string.h>

#define READONLY 1
#define WRITEONLY 2
#define READWRITE 3
#define APPEND 4

extern List *open_files;

#endif