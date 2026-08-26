/*
 * Intrusive free list with first fit properties.
 */

#include "allocator/freelist.h"
#include "debug/assert.h"
#include "error.h"
#include "string.h"

static void coalesce(FreeListBlockEntry *left, FreeListBlockEntry *right);

void freelist_init(FreeList *freelist, void *pool, const usize size)
{
	DEBUG("pool = %p, size = %d", pool, size);
	memset(freelist, 0, sizeof(FreeList));
	freelist->head = (FreeListBlockEntry *)pool;
	freelist->head->size = size;
	freelist->capacity = size;
}

void *freelist_alloc(FreeList *freelist, const usize size)
{
	TRACE("alloc pool = %p, size = %d", freelist->head, size);
	FreeListBlockEntry *entry = freelist->head;
	ASSERT(nullptr != entry, "head is null");
	ASSERT(0 != size, "allocation size is zero");

	usize actual_size = size + sizeof(FreeListBlockEntry);

	FreeListBlockEntry *prev = nullptr;
	FreeListBlockEntry *curr = freelist->head;

	while (nullptr != curr) {
		if (curr->size >= actual_size) {
			break;
		}

		prev = curr;
		curr = curr->next;
	}

	if (nullptr == curr) {
		return ERR_TO_PTR(-ENOMEM);
	}

	usize remain = curr->size - actual_size;

	/*
	 * Split only if another block can fit.
	 */
	if (remain >= sizeof(FreeListBlockEntry)) {
		FreeListBlockEntry *new =
			(FreeListBlockEntry *)((usize)curr + actual_size);

		new->size = remain;
		new->next = curr->next;

		if (prev) {
			prev->next = new;
		} else {
			freelist->head = new;
		}

		curr->size = actual_size;

	} else {
		/*
		 * Consume entire block.
		 */
		if (prev) {
			prev->next = curr->next;
		} else {
			freelist->head = curr->next;
		}
	}

	return (u8 *)curr + sizeof(FreeListBlockEntry);
}

void freelist_free(FreeList *freelist, void *ptr)
{
	TRACE("free pool = %p, ptr = %p", freelist->head, ptr);
	FreeListBlockEntry *current = freelist->head;
	ASSERT(nullptr != current, "head is null");
	ASSERT(nullptr != ptr, "ptr is null");

	FreeListBlockEntry *entry =
		(FreeListBlockEntry *)((usize)ptr - sizeof(FreeListBlockEntry));

	/*
	 * Insert at head.
	 */
	if (nullptr == freelist->head || entry < freelist->head) {
		entry->next = freelist->head;
		freelist->head = entry;

		coalesce(entry, entry->next);

		return;
	}

	/*
	 * otherwise
	 */
	FreeListBlockEntry *curr = freelist->head;

	while (nullptr != curr->next && curr->next < entry) {
		curr = curr->next;
	}

	entry->next = curr->next;
	curr->next = entry;

	coalesce(entry, entry->next);
	coalesce(curr, entry);
}

void freelist_add_region(FreeList *freelist, void *pool, usize size)
{
	ASSERT(nullptr != pool, "pool is null");
	ASSERT(0 != size, "size is zero");

	FreeListBlockEntry *entry = pool;
	entry->size = size;

	freelist->capacity += size;

	/* insert at begining */
	if (nullptr == freelist->head || entry < freelist->head) {
		entry->next = freelist->head;
		freelist->head = entry;
		coalesce(entry, entry->next);
		return;
	}

	FreeListBlockEntry *curr = freelist->head;
	while (nullptr != curr->next && entry > curr->next) {
		curr = curr->next;
	}

	entry->next = curr->next;
	curr->next = entry;
	coalesce(entry, entry->next);
	coalesce(curr, entry);
}

static void coalesce(FreeListBlockEntry *left, FreeListBlockEntry *right)
{
	if (nullptr == left || nullptr == right) {
		return;
	}

	if ((u8 *)left + left->size != (u8 *)right) {
		return;
	}

	left->size += right->size;
	left->next = right->next;
}
