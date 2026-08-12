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

/*
 * return
 * true if a is smaller than b
 * else false
 */
typedef bool (*comparator_fn_type)(IntrusiveNode *a, IntrusiveNode *b);

bool intrusivelist_node_is_null(IntrusiveNode *in);
void intrusivelist_node_init(IntrusiveNode *in);
void intrusivelist_init(IntrusiveList *il);
bool intrusivelist_insert_sorted(IntrusiveList *il, IntrusiveNode *node,
				 comparator_fn_type fn);
void intrusivelist_insert_head(IntrusiveList *il, IntrusiveNode *node);
void intrusivelist_insert_tail(IntrusiveList *il, IntrusiveNode *node);
bool intrusivelist_insert_at(IntrusiveList *il, IntrusiveNode *node,
			     usize index);
void intrusivelist_remove(IntrusiveList *il, IntrusiveNode *node);
IntrusiveNode *intrusivelist_remove_head(IntrusiveList *il);
IntrusiveNode *intrusivelist_remove_tail(IntrusiveList *il);
IntrusiveNode *intrusivelist_peek_head(const IntrusiveList *il);
IntrusiveNode *intrusivelist_peek_tail(const IntrusiveList *il);
usize intrusivelist_count(const IntrusiveList *il);
bool intrusivelist_is_empty(const IntrusiveList *il);

#define intrusivelist_foreach(list, node) \
	for ((node) = (list)->head; (node); (node) = (node)->next)

#endif // _INTRUSIVE_LIST_H_
