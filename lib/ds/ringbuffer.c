#include "ds/ringbuffer.h"
#include "error.h"
#include <string.h>

void ringbuffer_init(RingBuffer *rb, usize element_size, void *buffer,
		     usize buffer_size)
{
	rb->element_size = element_size;
	rb->head = 0;
	rb->tail = 0;
	rb->buffer = buffer;
	rb->capacity = buffer_size;
}

errno_t ringbuffer_push(RingBuffer *rb, void *data)
{
	/* next is where head will point to after this write */
	usize next = (rb->head + 1) % rb->capacity;

	/* if the head + 1 == tail, circular buffer is full */
	if (next == rb->tail) {
		return -ENOMEM;
	}

	memcpy(&rb->buffer[rb->head * rb->element_size], data,
	       rb->element_size);
	rb->head = next;
	return EOK;
}

errno_t ringbuffer_pop(RingBuffer *rb, void *data)
{
	/* if the head == tail, we dont have any data */
	if (rb->head == rb->tail) {
		return -ENOMEM;
	}

	memcpy(data, &rb->buffer[rb->tail * rb->element_size],
	       rb->element_size);

	rb->tail = (rb->tail + 1) % rb->capacity;

	return EOK;
}

bool ringbuffer_full(const RingBuffer *rb)
{
	return ((rb->head + 1) % rb->capacity) == rb->tail;
}

bool ringbuffer_empty(const RingBuffer *rb)
{
	return rb->head == rb->tail;
}
