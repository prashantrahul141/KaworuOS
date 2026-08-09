#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

/*
 * Generic linked list implementation
 *
 * Allocates mmeory using kernel heap and frees them.
 * It is the owner's job to manage object's lifetime.
 * Linkedlist does not hold the ownership of the objects it holds.
 */

#include "types.h"

typedef struct Node Node;

struct Node {
	void *data;
	Node *next;
	Node *prev;
};

typedef struct {
	Node *head;
	Node *tail;
	usize count;
} LinkedList;

void linkedlist_init(LinkedList *ll);

usize linkedlist_count(const LinkedList *ll);

void linkedlist_insert_head(LinkedList *ll, void *data);

void linkedlist_insert_tail(LinkedList *ll, void *data);

void *linkedlist_remove_head(LinkedList *ll);

void *linkedlist_remove_tail(LinkedList *ll);

void *linkedlist_peek_head(const LinkedList *ll);

void *linkedlist_peek_tail(const LinkedList *ll);

#endif // _LINKED_LIST_H_
