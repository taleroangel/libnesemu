#include "nesemu/memory/memory.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <stdint.h>

nesemu_error_t nes_mem_w8(nes_memory_t mem, uint16_t addr, uint8_t data)
{
    mem[addr] = data;
	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_r8(nes_memory_t mem, uint16_t addr, uint8_t *result)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (result == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	}
#endif

	*result = mem[addr];
	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_w16(nes_memory_t mem, uint16_t addr, uint16_t data)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (addr == (NESEMU_MEMORY_SIZE - 1)) {
		return NESEMU_RETURN_MEMORY_INVALILD_ADDR;
	}
#endif
    // LSB
	mem[addr] = (uint8_t)((data & 0xFF00) >> 8);
    // MSB
	mem[addr + 1] = (uint8_t)(data & 0x00FF);
	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_mem_r16(nes_memory_t mem, uint16_t addr, uint16_t *result)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (result == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	}
	if (addr == (NESEMU_MEMORY_SIZE - 1)) {
		return NESEMU_RETURN_MEMORY_INVALILD_ADDR;
	}
#endif
    // LSB
	*result = mem[addr];
    // MSB 
	*result |= (uint16_t)mem[addr + 1] << 8;
	return NESEMU_RETURN_SUCCESS;
}
