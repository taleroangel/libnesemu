#include "nesemu/memory/main.h"

#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* -- Private Functions -- */

/**
 * Syntax sugar around cartridge reader
 */
static inline nesemu_return_t _cartridge_read(struct nes_mem_main *mem,
					      uint16_t addr,
					      uint8_t *value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (mem->cartridge->prg_read_fn == NULL) {
		return NESEMU_RETURN_CARTRIDGE_NO_CALLBACK;
	}
#endif
	return mem->cartridge->prg_read_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(mem->cartridge), addr,
		value);
}

/**
 * Syntax sugar around cartridge writer
 */
static inline nesemu_return_t _cartridge_write(struct nes_mem_main *mem,
					       uint16_t addr,
					       uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (mem->cartridge->prg_write_fn == NULL) {
		return NESEMU_RETURN_CARTRIDGE_PRGROM_READ_ONLY;
	}
#endif
	return mem->cartridge->prg_write_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(mem->cartridge), addr,
		value);
}

/* -- Public Functions -- */

inline nesemu_return_t nes_mem_init(struct nes_mem_main *self,
				    struct nes_cartridge *cartridge)
{
	memset(self, 0, sizeof(struct nes_mem_main));
	self->cartridge = cartridge;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_mem_w8(struct nes_mem_main *self,
			   uint16_t addr,
			   uint8_t data)
{
	// Cartridge address, delegate to cartridge callback
	if (addr >= NESEMU_MEMORY_RAM_CARTRIDGE_BEGIN) {
		// ! Must return, the callback should handle the logic
		return _cartridge_write(self, addr, data);
	}

	//! Change the address based on mirroring rules

	// RAM mirroring
	else if ((addr >= NESEMU_MEMORY_RAM_MIRRORING_RANGE_START) &&
		 (addr <= NESEMU_MEMORY_RAM_MIRRORING_RANGE_END)) {
		// Address modulo with base should give the target address
		addr = addr % NESEMU_MEMORY_RAM_MIRRORING_BASE;
	}

	// PPU mirroring
	else if ((addr >= NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_RANGE_START) &&
		 (addr <= NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_RANGE_END)) {
		// Address modulo with base + addr should give the target address
		addr = (addr % NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_BASE) +
		       NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_ADDR;
	}

	// Write data to target address
	self->_data[addr] = data;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_mem_r8(struct nes_mem_main *self,
			   uint16_t addr,
			   uint8_t *result)
{
	// Cartridge address, delegate to cartridge callback
	if (addr >= NESEMU_MEMORY_RAM_CARTRIDGE_BEGIN) {
		// ! Must return, the callback should handle the logic
		return _cartridge_read(self, addr, result);
	}

	//! Change the address based on mirroring rules

	// RAM mirroring
	else if ((addr >= NESEMU_MEMORY_RAM_MIRRORING_RANGE_START) &&
		 (addr <= NESEMU_MEMORY_RAM_MIRRORING_RANGE_END)) {
		// Address modulo with base should give the target address
		addr = addr % NESEMU_MEMORY_RAM_MIRRORING_BASE;
	}

	// PPU mirroring
	else if ((addr >= NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_RANGE_START) &&
		 (addr <= NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_RANGE_END)) {
		// Address modulo with base + addr should give the target address
		addr = (addr % NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_BASE) +
		       NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_ADDR;
	}

	// Read data to target address
	*result = self->_data[addr];

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_mem_w16(struct nes_mem_main *self,
			    uint16_t addr,
			    uint16_t data)
{
	// Decompose u16 into two u8
	uint8_t msb = (data & 0xFF00) >> 8, lsb = (data & 0x00FF);

	// Write LSB
	nesemu_return_t err;
	if ((err = nes_mem_w8(self, addr, lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write next (MSB)
	if ((err = nes_mem_w8(self, addr + 1, msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_mem_r16(struct nes_mem_main *self,
			    uint16_t addr,
			    uint16_t *result)
{
	// Get the two placeholder bytes
	uint8_t lsb, msb;

	// Get LSB
	nesemu_return_t err;
	if ((err = nes_mem_r8(self, addr, &lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get next (MSB)
	if ((err = nes_mem_r8(self, addr + 1, &msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Build u16 from two u8
	*result = NESEMU_UTIL_U16(msb, lsb);

	return NESEMU_RETURN_SUCCESS;
}
