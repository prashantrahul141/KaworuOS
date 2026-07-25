#ifndef _REG_H_
#define _REG_H_

#include "debug/assert.h"
#include "types.h"

typedef struct {
	usize size;
	void *address;
} Reg;

static inline void _assert_reg_size(const Reg *reg, usize byte_offset)
{
	ASSERT(reg->size > byte_offset, "byte offset is larger than register "
					"size");
}

static inline u64 reg_read64(const Reg *reg, usize byte_offset)
{
	_assert_reg_size(reg, byte_offset);
	volatile u64 *addr = (void *)((u8 *)reg->address + byte_offset);
	return *addr;
}

static inline void reg_write64(const Reg *reg, usize byte_offset, u64 data)
{
	_assert_reg_size(reg, byte_offset);
	volatile u64 *addr = (void *)((u8 *)reg->address + byte_offset);
	*addr = data;
}

static inline u32 reg_read32(const Reg *reg, usize byte_offset)
{
	_assert_reg_size(reg, byte_offset);
	volatile u32 *addr = (void *)((u8 *)reg->address + byte_offset);
	return *addr;
}

static inline void reg_write32(const Reg *reg, usize byte_offset, u32 data)
{
	_assert_reg_size(reg, byte_offset);
	volatile u32 *addr = (void *)((u8 *)reg->address + byte_offset);
	*addr = data;
}

static inline u8 reg_read8(const Reg *reg, usize byte_offset)
{
	_assert_reg_size(reg, byte_offset);
	volatile u8 *addr = (void *)((u8 *)reg->address + byte_offset);
	return *addr;
}

static inline void reg_write8(const Reg *reg, usize byte_offset, u8 data)
{
	_assert_reg_size(reg, byte_offset);
	volatile u8 *addr = (void *)((u8 *)reg->address + byte_offset);
	*addr = data;
}

#endif // _REG_H_
