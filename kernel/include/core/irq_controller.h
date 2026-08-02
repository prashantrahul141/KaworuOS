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
 * set handler but do not enable irqs
 */
void irq_set_handler(u32 irq, irq_handler_t handler, void *data);

/*
 * enables irqs
 */
void irq_enable_irq(u32 irq);

/* enable irqs for this cpu */
void irq_enable_cpu(void);

/*
 * request irqs
 */
errno_t request_irq(u32 irq, irq_handler_t handler, void *data);

/*
 * stop irqs
 */
errno_t reject_irq(u32 irq);

#endif // _IRQ_CONTROLLER_H_
