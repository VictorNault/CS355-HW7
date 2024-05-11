// Name: node.h
// Purpose: Struct definition and function declaration for node.c
// Author: Victor Nault
// Date: 12/11/23

#ifndef NODE_H
#define NODE_H

struct node {
    void * data;
    struct node *next;
};

struct node *newNode(void * item);

#endif
