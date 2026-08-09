#ifndef _ARENA_H_
#define _ARENA_H_

#include "types.h"

typedef struct {
	u8 *pool;
	usize count;
	usize capacity;
} Arena;

/*
 * init an arena
 */
void arena_init(Arena *arena, void *pool, usize capacity);

/*
 * alloc from arena
 */
void *arena_alloc(Arena *arena, usize size);

/*
 * reset arena
 */
void arena_reset(Arena *arena);

/*
 * is full
 */
bool arena_is_full(const Arena *arena);

/* count and capacity */
usize arena_count(const Arena *arena);

usize arena_capacity(const Arena *arena);

/*
 * is empty
 */
bool arena_is_empty(const Arena *arena);

/*
 * amount of mem left
 */
usize arena_left(const Arena *arena);

/*
 * wrapper around, arena left to check if it can fit an allocation size
 */
bool arena_can_fit(const Arena *arena, usize size);

/*
 * returns pool of arena
 * since the arena doesnt know where the callee got the pool of memory
 * from, it is impossible for the arena to free it. So instead we can
 * use this to return the base address and then the callee can free it.
 */
void *arena_base(const Arena *arena);

#endif // _ARENA_H_
