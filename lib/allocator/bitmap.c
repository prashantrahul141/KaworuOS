#include "allocator/bitmap.h"
#include "common_defs.h"
#include "debug/log.h"
#include "error.h"
#include "memlayout.h"
#include "debug/assert.h"
#include "string.h"
#include <stdint.h>

static inline void *bm_allocate(AllocBitMap *alloc, usize page,
				usize page_count);
static inline void bm_free(AllocBitMap *alloc, void *addr, usize page_count);
static inline bool bm_is_page_allocated(const AllocBitMap *alloc, usize page);
static inline usize bm_find_n_free_pages(const AllocBitMap *alloc,
					 usize page_count);
static inline void bm_set_page_allocate(AllocBitMap *alloc, usize page);
static inline void bm_set_page_free(AllocBitMap *alloc, usize page);
static inline bool bm_get_page_allocated(const AllocBitMap *alloc, usize page);
static inline void *bm_page_to_addr(const AllocBitMap *alloc, usize page);
static inline usize bm_addr_to_page(const AllocBitMap *alloc, void *addr);

void *bitmap_alloc(AllocBitMap *alloc, usize page_count)
{
	TRACE("alloc alloc->pool = %p, page_count = %d", alloc->pool,
	      page_count);
	usize free_n_pos = bm_find_n_free_pages(alloc, page_count);
	if (SIZE_MAX == free_n_pos) {
		return ERR_TO_PTR(-ENOMEM);
	}

	return bm_allocate(alloc, free_n_pos, page_count);
}

/*
 * mark the given range of pages as allocated, without allocating from the
 * pool
 */
errno_t bitmap_reserve(AllocBitMap *alloc, void *addr, usize page_count)
{
	usize page = bm_addr_to_page(alloc, addr);
	for (usize pos = page; pos < page + page_count; pos++) {
		if (bm_is_page_allocated(alloc, pos)) {
			WARN("trying to reserve already allocated pages, addr "
			     "= %p",
			     addr);
			return -ENOMEM;
		}
	}

	bm_allocate(alloc, page, page_count);
	return EOK;
}

void bitmap_free(AllocBitMap *alloc, void *addr, usize page_count)
{
	TRACE("freeing alloc->pool = %p, page_count = %d", alloc->pool,
	      page_count);
	bm_free(alloc, addr, page_count);
}

/* copy allocation state from src to dst */
errno_t bitmap_copy(AllocBitMap *dst, const AllocBitMap *src)
{
	if (dst->page_count < src->page_count) {
		WARN("tried to copy bitmap but destination has less memory "
		     "than source");
		return -ENOMEM;
	}

	memset(dst->bitmap, 0,
	       SIZE_TO_BITMAP_BYTES(dst->page_count * PAGE_SIZE));

	for (usize page_i = 0; page_i < src->page_count; page_i++) {
		if (bm_is_page_allocated(src, page_i)) {
			bm_set_page_allocate(dst, page_i);
		}
	}

	return EOK;
}

void bitmap_init_alloc(AllocBitMap *alloc, void *pool, usize pool_size)
{
	ASSERT(IS_PAGE_ALIGNED(pool_size), "pool_size is not page aligned");

	usize bitmap_bytes = SIZE_TO_BITMAP_BYTES(pool_size);

	alloc->bitmap = (u8 *)round_down((usize)pool + pool_size - bitmap_bytes,
					 PAGE_SIZE);
	alloc->page_count = ((usize)alloc->bitmap - (usize)pool) / PAGE_SIZE;
	alloc->pool = (u8 *)pool;

	ASSERT(alloc->page_count > 0, "pool too small to hold bitmap");

	memset(alloc->bitmap, 0, bitmap_bytes);
}

static inline usize bm_find_n_free_pages(const AllocBitMap *alloc,
					 usize page_count)
{
	TRACE("finding n = %d free contigious pages from pool = %p", page_count,
	      alloc->pool);
	for (usize start = 0; start + page_count <= alloc->page_count;) {
		usize i = 0;
		for (; i < page_count; i++) {
			if (bm_is_page_allocated(alloc, start + i)) {
				break;
			}
		}

		if (i == page_count) {
			return start;
		}

		start += i + 1;
	}

	return SIZE_MAX;
}

static inline bool bm_is_page_allocated(const AllocBitMap *alloc, usize page)
{
	ASSERT(alloc->page_count > page, "checking page larger than total "
					 "pages");
	ASSERT(page >= 0, "page is less than zero?");
	return bm_get_page_allocated(alloc, page);
}

static inline void *bm_allocate(AllocBitMap *alloc, usize page,
				usize page_count)
{
	ASSERT(alloc->page_count > page, "page larger than total "
					 "pages");
	ASSERT(alloc->page_count >= page + page_count, "page + size larger "
						       "than total pages");
	ASSERT(page >= 0, "page index is less than zero?");
	ASSERT(page_count > 0, "size is less than or equal to zero?");
	for (usize pos = page; pos < page + page_count; pos++)
		bm_set_page_allocate(alloc, pos);
	return bm_page_to_addr(alloc, page);
}

static inline void bm_free(AllocBitMap *alloc, void *addr, usize page_count)
{
	usize page = bm_addr_to_page(alloc, addr);
	ASSERT(alloc->page_count > page, "checking page larger than "
					 "total pages");
	ASSERT(alloc->page_count >= page + page_count, "page + size larger "
						       "than total pages");
	ASSERT(page >= 0, "page index is less than zero?");
	ASSERT(page_count > 0, "size is less than or equal to zero?");

	for (usize pos = page; pos < page + page_count; pos++) {
		if (!bm_get_page_allocated(alloc, pos)) {
			panic("freeing pages which arent allocated page = %d",
			      pos);
		}
	}

	for (usize pos = page; pos < page + page_count; pos++) {
		bm_set_page_free(alloc, pos);
	}
}

static inline void *bm_page_to_addr(const AllocBitMap *alloc, usize page)
{
	return alloc->pool + (page * PAGE_SIZE);
}

static inline usize bm_addr_to_page(const AllocBitMap *alloc, void *addr)
{
	ASSERT((u8 *)addr >= alloc->pool, "addr smaller than pool");
	ASSERT((u8 *)addr < alloc->pool + (alloc->page_count * PAGE_SIZE),
	       "addr larger than max pool");
	return ((usize)((u8 *)addr - alloc->pool) / PAGE_SIZE);
}

static inline void bm_set_page_allocate(AllocBitMap *alloc, usize page)
{
	usize byte_offset = (page / 8);
	usize bit_offset = (page % 8);
	alloc->bitmap[byte_offset] =
		(u8)SET_BIT(alloc->bitmap[byte_offset], bit_offset);
}

static inline void bm_set_page_free(AllocBitMap *alloc, usize page)
{
	usize byte_offset = (page / 8);
	usize bit_offset = (page % 8);
	alloc->bitmap[byte_offset] =
		(u8)CLEAR_BIT(alloc->bitmap[byte_offset], bit_offset);
}

static inline bool bm_get_page_allocated(const AllocBitMap *alloc, usize page)
{
	usize byte_offset = (page / 8);
	usize bit_offset = (page % 8);
	return (bool)GET_BIT(alloc->bitmap[byte_offset], bit_offset);
}
