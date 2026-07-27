#ifndef _IRQ_CONTROLLER_H_
#define _IRQ_CONTROLLER_H_

#include "error.h"

typedef void (*irq_handler_t)(void *data);

typedef struct {
	irq_handler_t handler;
	void *data;
} irq_desc;

void irq_controller_init(void);

errno_t request_irq(u32 irq, irq_handler_t handler, void *data);

void irq_dispatcher(void);

#endif // _IRQ_CONTROLLER_H_
