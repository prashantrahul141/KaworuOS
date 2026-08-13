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

bool linkedlist_insert_at(LinkedList *ll, void *data, usize index)
{
	if (index == 0) {
		linkedlist_insert_head(ll, data);
		return true;
	}

	if (index == ll->count) {
		linkedlist_insert_tail(ll, data);
		return true;
	}

	if (ll->count < index) {
		return false;
	}

	Node *existing;
	usize counting = 0;
	linkedlist_foreach(ll, existing) {
		if (counting == index) {
			Node *new_node = kalloc(sizeof(Node));

			/* right side */
			new_node->next = existing;
			Node *was_prev = existing->prev;
			existing->prev = new_node;

			/* left side */
			new_node->prev = was_prev;
			was_prev->next = new_node;

			new_node->data = data;

			ll->count++;
			break;
		}

		counting++;
	}

	return counting == index;

	return true;
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

bool linkedlist_remove(LinkedList *ll, void *data)
{
	Node *node;
	linkedlist_foreach(ll, node) {
		if (node->data != data) {
			continue;
		}

		if (nullptr != node->prev) {
			node->prev->next = node->next;
		} else {
			ll->head = node->next;
		}

		if (nullptr != node->next) {
			node->next->prev = node->prev;
		} else {
			ll->tail = node->prev;
		}

		ll->count--;
		kfree(node);
		return true;
	}

	return false;
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
