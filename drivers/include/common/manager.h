#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "error.h"
#include "io/io.h"
#include "types.h"

typedef struct Device Device;

typedef enum : isize {
	DEVICE_UNKNOWN = -1,
	DEVICE_FRAMEBUFFER,
	DEVICE_UART,
	DEVICE_IRQCHIP
} DeviceClass;

typedef struct {
	const i8 *name;
	const i8 *const *compatible;
	errno_t (*probe)(Device *);
	errno_t (*remove)(Device *);
	DeviceClass device_class;
} Driver;

typedef struct {
	void (*write)(Device *device, const IOEvent *event);
	u8 (*read)(Device *device);
} DriverOps;

typedef struct {
	void (*write)(Device *backend, const IOEvent *event);
	u8 (*read)(Device *backend);
	void (*flush)(Device *backend);
} ConsoleOps;

typedef struct {
	void (*enable)(const Device *device, u32 irq);
	void (*disable)(const Device *device, u32 irq);
	u32 (*get_active)(const Device *device);
	void (*signal_eoi)(const Device *device, u32 irq);
	u32 (*interrupts_count)(const Device *device);
} IrqChipOps;

typedef enum {
	DEVICE_DISCOVERED = 0,
	DEVICE_READY,
	DEVICE_FAILED,
} DeviceState;

struct Device {
	const i8 *name;
	const Driver *driver;
	DeviceState state;
	union {
		const ConsoleOps *console_ops;
		const IrqChipOps *irq_chip_ops;
	};
	void *driver_data;
	i32 fdt_node_offset;
	Device *next;
};

#define REGISTER_DEVICE_DRIVER(driver) \
	USED SECTION(".driver") const Driver *driver##_ptr = &(driver)

#define ACCESS_DRIVER_DATA(type, device) ((type *)((device)->driver_data))

/*
 * Initialize driver manager
 */
void dmanager_init(void);

/*
 * ready a device for usage
 */
errno_t dmanager_ready_device(Device *device);

/*
 * Get first device with a specific class
 */
Device *dmanager_get_by_class(DeviceClass class);

/*
 * wrapper around dmanager_get_by_class and dmanager_ready_device
 */
Device *dmanager_get_by_class_and_ready(DeviceClass class);

#endif // _PLATFORM_H_
