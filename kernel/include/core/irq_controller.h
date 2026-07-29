#ifndef _IRQ_CONTROLLER_H_
#define _IRQ_CONTROLLER_H_

#include "error.h"

typedef void (*irq_handler_t)(void *data);

typedef struct {
	irq_handler_t handler;
	void *data;
} irq_desc;

void irq_controller_init(void);

void irq_dispatcher(void);

/*
 * request irqs
 */
errno_t request_irq(u32 irq, irq_handler_t handler, void *data);

/*
 * stop irqs
 */
errno_t reject_irq(u32 irq);

#endif // _IRQ_CONTROLLER_H_
