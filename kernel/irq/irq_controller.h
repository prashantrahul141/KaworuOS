#ifndef _IRQ_CONTROLLER_H_
#define _IRQ_CONTROLLER_H_

#include "aarch64/exception.h"
#include "error.h"

typedef void (*irq_handler_t)(void *data);

typedef struct {
	irq_handler_t handler;
	void *data;
} irq_desc;

void irq_controller_init(void);

/*
 * Handler for all interrupts
 *
 * returns whether cpu needs resched
 */
bool irq_handle(ExceptionFrame *frame);

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

/* push and pop like toggle for interrupts */

/* very similar to w_intrd_disable, w_intrd_enable but works like a stack. */
void irq_push_intr(void);
void irq_pop_intr(void);

/* check if local irq is enabled */
bool irq_local_is_enable(void);

/* disable local irqs */
void irq_local_disable(void);

/* enable local irqs */
void irq_local_enable(void);

#endif // _IRQ_CONTROLLER_H_
