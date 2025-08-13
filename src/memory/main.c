#include "nesemu/memory/main.h"
#include "nesemu/cartridge/types/common.h"

#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

inline nesemu_error_t nes_mem_reset(struct nes_main_memory_t *mem)
{
	memset(mem, 0, sizeof(struct nes_main_memory_t));
	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_w8(struct nes_main_memory_t *mem,
			  uint16_t addr,
			  uint8_t data)
{
	// Cartridge address, delegate to cartridge callback
	if (addr >= NESEMU_CARTRIDGE_ADDR_BEGIN) {
		// ! Must return, the callback should handle the logic
		return nes_mem_cartridge_write(mem, addr, data);
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
	mem->_data[addr] = data;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_r8(struct nes_main_memory_t *mem,
			  uint16_t addr,
			  uint8_t *result)
{
	// Cartridge address, delegate to cartridge callback
	if (addr >= NESEMU_CARTRIDGE_ADDR_BEGIN) {
		// ! Must return, the callback should handle the logic
		return nes_mem_cartridge_read(mem, addr, result);
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
	*result = mem->_data[addr];

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_w16(struct nes_main_memory_t *mem,
			   uint16_t addr,
			   uint16_t data)
{
	// Decompose u16 into two u8
	uint8_t msb = (data & 0xFF00) >> 8, lsb = (data & 0x00FF);

	// Write LSB
	nesemu_error_t err;
	if ((err = nes_mem_w8(mem, addr, lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write next (MSB)
	if ((err = nes_mem_w8(mem, addr + 1, msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_r16(struct nes_main_memory_t *mem,
			   uint16_t addr,
			   uint16_t *result)
{
	// Get the two placeholder bytes
	uint8_t lsb, msb;

	// Get LSB
	nesemu_error_t err;
	if ((err = nes_mem_r8(mem, addr, &lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get next (MSB)
	if ((err = nes_mem_r8(mem, addr + 1, &msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Build u16 from two u8
	*result = NESEMU_UTIL_U16(msb, lsb);

	return NESEMU_RETURN_SUCCESS;
}
