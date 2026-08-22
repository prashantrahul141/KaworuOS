#include "io/console.h"
#include "allocator/arena.h"
#include "debug/assert.h"
#include "debug/log.h"
#include "ds/ringbuffer.h"
#include "error.h"
#include "io/io.h"
#include "mm/kheap.h"
#include "sync/spinlock.h"
#include "string.h"

constexpr usize _IO_EVENT_BUFFER_COUNT = 5;
constexpr usize _IO_EVENT_MSG_SIZE_ESTIMATE = 100;
constexpr usize CONSOLE_RECV_RING_BUFFER_SIZE = 1024;

constexpr usize IO_EVENT_BUFFER_SIZE = sizeof(IOEvent) * _IO_EVENT_BUFFER_COUNT;
constexpr usize IO_EVENT_MSG_BUFFER_SIZE =
	_IO_EVENT_BUFFER_COUNT * _IO_EVENT_MSG_SIZE_ESTIMATE;

typedef struct {
	SpinLock write_lock;
	SpinLock read_lock;
	ConsoleBackend *backends;
	RingBuffer recv_rb;
	Arena io_events;
	Arena io_messages;
	bool enable_buffering;
} Console;

static Console console = {
	.backends = nullptr,
};

static u8 io_event_buffer_storage[IO_EVENT_BUFFER_SIZE];
static u8 io_event_msg_buffer[IO_EVENT_MSG_BUFFER_SIZE];
static u8 console_recv_buffer[CONSOLE_RECV_RING_BUFFER_SIZE];

errno_t console_init(bool buffering)
{
	spinlock_init(&console.write_lock, "console - write");
	spinlock_init(&console.read_lock, "console - read");
	arena_init(&console.io_events, io_event_buffer_storage,
		   sizeof(io_event_buffer_storage));
	arena_init(&console.io_messages, io_event_msg_buffer,
		   sizeof(io_event_msg_buffer));
	ringbuffer_init(&console.recv_rb, sizeof(u8), console_recv_buffer,
			CONSOLE_RECV_RING_BUFFER_SIZE);
	console.enable_buffering = buffering;
	return EOK;
}

void console_deinit()
{
}

void console_register_backend(ConsoleBackend *backend)
{
	DEBUG("registering device = %s", backend->device->name);
	spinlock_acquire_scoped(&console.write_lock);
	backend->next = console.backends;
	console.backends = backend;
}

void console_register(Device *device)
{
	ConsoleBackend *backend = kalloc(sizeof(ConsoleBackend));
	backend->device = device;
	console_register_backend(backend);
}

bool console_unregister(const Device *device)
{
	DEBUG("removing device = %s", device->name);
	spinlock_acquire_scoped(&console.write_lock);
	ConsoleBackend *curr = console.backends;
	ConsoleBackend *prev = nullptr;
	while (nullptr != curr) {
		/* found backend */
		if (curr->device == device) {
			if (prev == nullptr) {
				console.backends = curr->next;
			} else {
				prev->next = curr->next;
			}

			curr->next = nullptr;
			return true;
		}

		prev = curr;
		curr = curr->next;
	}

	return false;
}

static inline void write_to_all_backends(const IOEvent *ev)
{
	ConsoleBackend *backend = console.backends;
	while (nullptr != backend) {
		backend->device->console_ops->write(backend->device, ev);
		backend = backend->next;
	}
}

static inline void finalize_write(void)
{
	IOEvent *events = (IOEvent *)arena_base(&console.io_events);
	usize count = arena_count(&console.io_events) / sizeof(IOEvent);
	for (usize i = 0; i < count; i++) {
		write_to_all_backends(&events[i]);
	}
}

static bool can_fit(const IOEvent *ev)
{
	return arena_can_fit(&console.io_messages, ev->len) &&
	       arena_can_fit(&console.io_events, sizeof(IOEvent));
}

static void reset_buffers()
{
	arena_reset(&console.io_messages);
	arena_reset(&console.io_events);
}

static void write_event(IOEvent *ev)
{
	void *msg_alloc = arena_alloc(&console.io_messages, ev->len);
	ASSERT(!IS_ERR(msg_alloc), "failed to allocate for io message");
	memcpy(msg_alloc, ev->msg, ev->len);
	ev->msg = msg_alloc;
	void *event_alloc = arena_alloc(&console.io_events, sizeof(IOEvent));
	ASSERT(!IS_ERR(event_alloc), "failed to allocate for event");
	memcpy(event_alloc, ev, sizeof(IOEvent));
}

static errno_t _console_flush()
{
	finalize_write();
	ConsoleBackend *backend = console.backends;
	while (nullptr != backend) {
		backend->device->console_ops->flush(backend->device);
		backend = backend->next;
	}

	reset_buffers();
	return EOK;
}

errno_t console_flush()
{
	spinlock_acquire_scoped(&console.write_lock);
	_console_flush();
	return EOK;
}

static errno_t _console_write(IOEvent *e)
{
	if (!console.enable_buffering) {
		write_event(e);
		_console_flush();
		return EOK;
	}

	/* if it cant hold anymore, like myself */
	if (!can_fit(e)) {
		/* write, flush all messages & reset buffers */
		_console_flush();
	}

	write_event(e);
	return EOK;
}

errno_t console_write_multiple(IOEvent *events, usize events_count)
{
	spinlock_acquire_scoped(&console.write_lock);
	for (usize i = 0; i < events_count; i++) {
		errno_t ret = _console_write(&events[i]);
		if (EOK != ret) {
			return ret;
		}
	}
	return EOK;
}

errno_t console_write(IOEvent e)
{
	spinlock_acquire_scoped(&console.write_lock);
	return _console_write(&e);
}

void console_set_buffering(bool buffering)
{
	spinlock_acquire_scoped(&console.write_lock);
	console.enable_buffering = buffering;
}

errno_t console_write_char(i8 c)
{
	IOEvent e = io_event_default(&c, 1);
	return console_write(e);
}

errno_t console_receive_char(i8 c)
{
	spinlock_acquire_scoped(&console.read_lock);
	return ringbuffer_push(&console.recv_rb, &c);
}

bool console_read(u8 *out)
{
	spinlock_acquire_scoped(&console.read_lock);
	if (ringbuffer_empty(&console.recv_rb)) {
		return false;
	}
	ringbuffer_pop(&console.recv_rb, out);
	return true;
}
