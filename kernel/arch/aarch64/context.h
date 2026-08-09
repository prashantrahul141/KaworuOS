#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "types.h"
#include "common_defs.h"

typedef struct {
	u64 x19, x20, x21, x22, x23, x24, x25, x26, x27, x28,
		fp, // x29
		lr, // x30
		sp;
} ExecutionContext;

static_assert(sizeof(ExecutionContext) == 104, "ExecutionContext is not 104 "
					       "bytes?");

/* saves current cpu state to old context and switches to new context */
void context_switch(ExecutionContext *old, ExecutionContext *new);

#endif // _CONTEXT_H_
