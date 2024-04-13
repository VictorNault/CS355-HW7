// Name: List.h
// Purpose: Struct definition and function declarations for List.c
// Author: Victor Nault
// Date: 12/11/23

#ifndef LIST_H
#define LIST_H
typedef unsigned long tid_t;
typedef struct {
    int size;
    struct node *head;
    struct node * tail;
} List;

 void addHead(List *l, void *item);
 List * newList(); // Creates a new empty list
 int size(const List *l); // Returns the size of list-l
 int empty(const List *l); // is the list-l empty?
 void clear(List *l); // removes all items from list-l
 void clear_1(List *l) ;
 void clear_func(List *l);
 void add(List *l, void * item); // Add item at end of list-l
 void add_SJF(List *l, void * item);
 void deleteHead(List *l); // Delete the head element
 void * get(const List *l, int index); // Returns item at index in list-l
// No need for this in this implementation
/* int contains(const List *l, void * item); // Does list-l have item? */

#endif
