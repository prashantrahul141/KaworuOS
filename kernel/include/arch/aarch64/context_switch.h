#include "aarch64/context.h"

/* saves current cpu state to old context and switches to new context */
void context_switch(ExecutionContext *old, ExecutionContext *new);
