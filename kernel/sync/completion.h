#ifndef _COMPLETION_H_
#define _COMPLETION_H_

/*
 * completion are like semphores but they are binary
 */

#include "sync/wait_queue.h"

typedef struct {
	WaitQueue waiters;
	bool done;
} Completion;

void completion_init(Completion *cm, const i8 *name);

/*
 * signal waiting for a event
 * blocks current
 */
void completion_wait(Completion *cm);

/*
 * signal completion of the event
 * wake all blocked tasks
 */
void completion_signal(Completion *cm);

/*
 * rearms completion
 */
void completion_reset(Completion *cm);

#endif // _COMPLETION_H_
