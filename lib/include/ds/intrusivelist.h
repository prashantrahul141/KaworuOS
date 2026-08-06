#ifndef _INTRUSIVE_LIST_H_
#define _INTRUSIVE_LIST_H_

#include "types.h"
typedef struct IntrusiveNode IntrusiveNode;

struct IntrusiveNode {
	IntrusiveNode *next;
	IntrusiveNode *prev;
};

typedef struct {
	IntrusiveNode *head;
	IntrusiveNode *tail;
	usize count;
} IntrusiveList;

void intrusivelist_node_init(IntrusiveNode *in);
void intrusivelist_init(IntrusiveList *il);
void intrusivelist_insert_head(IntrusiveList *il, IntrusiveNode *node);
void intrusivelist_insert_tail(IntrusiveList *il, IntrusiveNode *node);
void intrusivelist_remove(IntrusiveList *il, IntrusiveNode *node);
IntrusiveNode *intrusivelist_remove_head(IntrusiveList *il);
IntrusiveNode *intrusivelist_remove_tail(IntrusiveList *il);
IntrusiveNode *intrusivelist_peek_head(const IntrusiveList *il);
IntrusiveNode *intrusivelist_peek_tail(const IntrusiveList *il);
usize intrusivelist_count(const IntrusiveList *il);
bool intrusivelist_is_empty(const IntrusiveList *il);

#endif // _INTRUSIVE_LIST_H_
