#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "error.h"
#include "io/io.h"
#include "types.h"

typedef struct Device Device;
typedef enum : isize {
	DEVICE_UNKNOWN = -1,
	DEVICE_FRAMEBUFFER,
	DEVICE_CONSOLE,
	DEVICE_IRQCHIP,
	DEVICE_TIMER,
	DEVICE_BLOCK,
	DEVICE_ENTROPY,
	DEVICE_NETWORK,
	DEVICE_VIRTIO_MMIO_DEVICE,
} DeviceClass;

typedef struct {
	/*
	 * driver name
	 */
	const i8 *name;

	/*
	 * compat array
	 */
	const i8 *const *compatible;

	/*
	 * init aka probe function
	 */
	errno_t (*probe)(Device *);

	/*
	 * removal or destroying function
	 */
	errno_t (*remove)(Device *);

	/*
	 * type of device
	 */
	DeviceClass device_class;
} Driver;

typedef struct {
	/*
	 * read one byte from the device
	 */
	u8 (*read)(Device *device);

	/*
	 * write an entire event to device
	 */
	void (*write)(Device *device, const IOEvent *event);

	/*
	 * flush device if it supports
	 */
	void (*flush)(Device *device);
} ConsoleOps;

typedef struct {
	/*
	 * read from given sector into buf
	 *
	 * buf should be atleast sector sized
	 */
	errno_t (*read)(Device *device, usize sector, void *buf);

	/*
	 * write given sector into buf
	 *
	 * buf should be atleast sector sized
	 */
	errno_t (*write)(Device *device, usize sector, const void *buf);

	/*
	 * returns sector size of the device, in bits
	 */
	usize (*sector_size)(Device *device);

	/*
	 * returns total capacity, in bits
	 */
	usize (*capacity)(Device *device);
} BlockOps;

typedef struct {
	/*
	 * init interrupt for this cpu
	 */
	void (*cpu_init)(Device *device);

	/*
	 * enable a given irq
	 */
	void (*enable)(const Device *device, u32 irq);

	/*
	 * disable a given irq
	 */
	void (*disable)(const Device *device, u32 irq);

	/*
	 * get currently active interrupt
	 */
	u32 (*get_active)(const Device *device);

	/*
	 * signal end of interrupt
	 */
	void (*signal_eoi)(const Device *device, u32 irq);

	/*
	 * total interrupt count
	 */
	u32 (*interrupts_count)(const Device *device);
} IrqChipOps;

typedef struct {
	/*
	 * Enable this timer
	 */
	void (*enable)(Device *device);
	/*
	 * Disable this timer
	 */
	void (*disable)(Device *device);
	/*
	 * Frequency of this timer
	 */
	usize (*frequency)(const Device *device);
	/*
	 * Current always incrementing counter value
	 */
	usize (*counter)(const Device *device);
	/*
	 * Sets an interrupt ticks from now
	 */
	void (*fire_from_now)(Device *device, usize ticks);
	/*
	 * Sets an interrupts at a given tick
	 */
	void (*fire_at)(Device *device, usize ticks);
	/*
	 * Interrupt id
	 */
	u32 (*interrupt_id)(Device *device);
} TimerOps;

typedef enum {
	/*
	 * deviec has been discovered but havent been probed yet
	 */
	DEVICE_DISCOVERED = 0,

	/*
	 * device was successfully probed and is ready to use
	 */
	DEVICE_READY,

	/*
	 * device was failed probing
	 */
	DEVICE_FAILED,
} DeviceState;

struct Device {
	const i8 *name;
	const Driver *driver;
	DeviceClass class;
	DeviceState state;
	union {
		const ConsoleOps *console_ops;
		const IrqChipOps *irq_chip_ops;
		const TimerOps *timer_ops;
		const BlockOps *block_ops;
	};
	void *driver_data;
	i32 fdt_node_offset;
	Device *next;
};

#define REGISTER_DEVICE_DRIVER(driver) \
	USED SECTION(".driver") const Driver *driver##_ptr = &(driver)

/*
 * driver: pointer to a pointer to a driver
 */
#define dmanager_foreach_driver(driver)                                   \
	for (Driver * *(driver) = (Driver **)__KERNEL_TEXT_DRIVERS_START; \
	     (driver) < (Driver **)__KERNEL_TEXT_DRIVERS_END; (driver)++)

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
Device *dmanager_get_by_class_first(DeviceClass class);

/*
 * get device after skipping similar n devices
 */
Device *dmanager_get_by_class_skip(DeviceClass class, usize skip);

/*
 * wrapper around dmanager_get_by_class and dmanager_ready_device
 */
Device *dmanager_get_by_class_and_ready(DeviceClass class);

#endif // _PLATFORM_H_
