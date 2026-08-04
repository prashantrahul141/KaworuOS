#include "ds/linkedlist.h"
#include "mm/kheap.h"

void linkedlist_init(LinkedList *ll)
{
	ll->count = 0;
	ll->head = nullptr;
	ll->tail = nullptr;
}

usize linkedlist_count(const LinkedList *ll)
{
	return ll->count;
}

void linkedlist_insert_head(LinkedList *ll, void *data)
{
	Node *node = kalloc(sizeof(Node));
	node->data = data;
	node->next = ll->head;
	node->prev = nullptr;

	if (nullptr != ll->head) {
		ll->head->prev = node;
	} else {
		ll->tail = node;
	}

	ll->head = node;
	ll->count++;
}

void linkedlist_insert_tail(LinkedList *ll, void *data)
{
	Node *node = kalloc(sizeof(Node));
	node->data = data;
	node->next = nullptr;
	node->prev = ll->tail;

	if (nullptr != ll->tail) {
		ll->tail->next = node;
	} else {
		ll->head = node;
	}

	ll->tail = node;
	ll->count++;
}

void *linkedlist_remove_head(LinkedList *ll)
{
	Node *current_head = ll->head;
	if (nullptr == current_head) {
		return nullptr;
	}

	ll->head = current_head->next;

	if (nullptr != ll->head) {
		ll->head->prev = nullptr;
	} else {
		ll->tail = nullptr;
	}

	void *data = current_head->data;

	kfree(current_head);
	ll->count--;

	return data;
}

void *linkedlist_remove_tail(LinkedList *ll)
{
	Node *current_tail = ll->tail;
	ll->tail = current_tail->prev;

	if (nullptr != ll->tail) {
		ll->tail->next = nullptr;
	} else {
		ll->head = nullptr;
	}

	void *data = current_tail->data;

	kfree(current_tail);
	ll->count--;

	return data;
}

void *linkedlist_peek_head(const LinkedList *ll)
{
	if (nullptr == ll->head) {
		return nullptr;
	}

	return ll->head->data;
}

void *linkedlist_peek_tail(const LinkedList *ll)
{
	if (nullptr == ll->tail) {
		return nullptr;
	}

	return ll->tail->data;
}
