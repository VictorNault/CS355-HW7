// Name: node.c
// Purpose: Holds the function to make a new node struct
// Author: Victor Nault
// Date: 12/11/23

#include <stdlib.h>
#include "node.h"
#include "common.h"

struct node *newNode(void * item) { // Creates a new node with item
    struct node *n = malloc(sizeof(struct node));
    n->data = item;
    n->next = NULL;
} // newNode()
