#include "nesemu/memory/memory.h"
#include "nesemu/memory/stack.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <stdint.h>

nesemu_error_t nes_stack_push(nes_memory_t mem, uint8_t *sp, uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (sp == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	} else if (*sp == 0x00) {
		return NESEMU_RETURN_MEMORY_STACK_OVERFLOW;
	}
#endif
	// Reduce the sp (descending stack)
	*sp -= 1;
	return nes_mem_w8(mem, NESEMU_STACK_GET_ADDR(*sp), value);
}

nesemu_error_t nes_stack_pop(nes_memory_t mem, uint8_t *sp, uint8_t *result)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (sp == NULL || result == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	} else if (*sp == NESEMU_STACK_SIZE) {
		return NESEMU_RETURN_MEMORY_STACK_UNDERFLOW;
	}
#endif
	// Increment the sp (descending stack)
	*sp += 1;
	// Get the value
	uint16_t addr = NESEMU_STACK_GET_ADDR(*sp);

    // Read value from stack
	nesemu_error_t rop = nes_mem_r8(mem, addr, result);
	// Remove from stack
    nesemu_error_t wop = nes_mem_w8(mem, addr, 0);
	
    return rop | wop;
}
