/* kernel's heap allocator for object sized allocations */

#ifndef _K_HEAP_H_
#define _K_HEAP_H_

#include "types.h"

void kheap_init(void);
void *kalloc(usize size);
void kfree(void *ptr);

#endif // _K_HEAP_H_
