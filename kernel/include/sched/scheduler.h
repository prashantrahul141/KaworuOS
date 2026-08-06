#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/*
 * switch task to the next READY one.
 */
void scheduler_switch(void);

/*
 * wrapper around scheduler_switch
 */
void yield(void);

#endif // _SCHEDULER_H_
