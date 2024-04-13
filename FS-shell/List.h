// Name: List.h
// Purpose: Struct definition and function declarations for List.c
// Author: Victor Nault
// Date: 12/11/23

#ifndef LIST_H
#define LIST_H

typedef struct {
    int size;
    struct node *head;
    struct node * tail;
} List;

List * newList(); // Creates a new empty list
int size(const List *l); // Returns the size of list-l
int empty(const List *l); // is the list-l empty?
void clear(List *l); // removes all items from list-l
void add(List *l, void * item); // Add item at end of list-l
void deleteHead(List *l); // Delete the head element
void * get(const List *l, int index); // Returns item at index in list-l
// No need for this in this implementation
/* int contains(const List *l, void * item); // Does list-l have item? */
void print(const List *l); // prints contents of list (test only)

#endif
