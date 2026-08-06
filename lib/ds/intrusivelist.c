#include "ds/intrusivelist.h"

void intrusivelist_node_init(IntrusiveNode *in)
{
	in->next = nullptr;
	in->prev = nullptr;
}

void intrusivelist_init(IntrusiveList *il)
{
	il->head = nullptr;
	il->tail = nullptr;
	il->count = 0;
}

void intrusivelist_insert_head(IntrusiveList *il, IntrusiveNode *node)
{
	node->next = il->head;
	node->prev = nullptr;

	if (nullptr != il->head) {
		il->head->prev = node;
	} else {
		il->tail = node;
	}

	il->count++;
	il->head = node;
}

void intrusivelist_insert_tail(IntrusiveList *il, IntrusiveNode *node)
{
	node->next = nullptr;
	node->prev = il->tail;

	if (nullptr != il->tail) {
		il->tail->next = node;
	} else {
		il->head = node;
	}

	il->tail = node;
	il->count++;
}

void intrusivelist_remove(IntrusiveList *il, IntrusiveNode *node)
{
	if (nullptr != node->prev) {
		node->prev->next = node->next;
	} else {
		il->head = node->next;
	}

	if (nullptr != node->next) {
		node->next->prev = node->prev;
	} else {
		il->tail = node->prev;
	}

	il->count--;
	node->next = nullptr;
	node->prev = nullptr;
}

IntrusiveNode *intrusivelist_remove_head(IntrusiveList *il)
{
	IntrusiveNode *current_head = il->head;
	if (nullptr == current_head) {
		return nullptr;
	}

	il->head = current_head->next;

	if (nullptr != il->head) {
		il->head->prev = nullptr;
	} else {
		il->tail = nullptr;
	}

	il->count--;
	current_head->next = nullptr;
	current_head->prev = nullptr;
	return current_head;
}

IntrusiveNode *intrusivelist_remove_tail(IntrusiveList *il)
{
	IntrusiveNode *current_tail = il->tail;
	il->tail = current_tail->prev;

	if (nullptr != il->tail) {
		il->tail->next = nullptr;
	} else {
		il->head = nullptr;
	}

	il->count--;
	current_tail->next = nullptr;
	current_tail->prev = nullptr;
	return current_tail;
}

IntrusiveNode *intrusivelist_peek_head(const IntrusiveList *il)
{
	if (nullptr == il->head) {
		return nullptr;
	}

	return il->head;
}

IntrusiveNode *intrusivelist_peek_tail(const IntrusiveList *il)
{
	if (nullptr == il->tail) {
		return nullptr;
	}

	return il->tail;
}

usize intrusivelist_count(const IntrusiveList *il)
{
	return il->count;
}

bool intrusivelist_is_empty(const IntrusiveList *il)
{
	return il->count == 0;
}

#define intrusivelist_foreach(list, node) \
	for ((node) = (list)->head; (node); (node) = (node)->next)
