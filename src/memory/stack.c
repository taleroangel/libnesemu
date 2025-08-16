#include "nesemu/memory/main.h"
#include "nesemu/memory/stack.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <stdint.h>

nesemu_return_t nes_stack_push_u8(struct nes_main_memory_t *mem,
				 uint8_t *sp,
				 uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (sp == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	} else if (*sp == 0x00) {
		return NESEMU_RETURN_MEMORY_STACK_OVERFLOW;
	}
#endif
	// Store value in stack
	nesemu_return_t err = nes_mem_w8(mem, NESEMU_STACK_GET_ADDR(*sp), value);
	// Reduce the sp (descending stack)
	*sp -= 1;
	return err;
}

nesemu_return_t nes_stack_pop_u8(struct nes_main_memory_t *mem,
				uint8_t *sp,
				uint8_t *result)
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
	return nes_mem_r8(mem, addr, result);
}

nesemu_return_t nes_stack_push_u16(struct nes_main_memory_t *mem,
				  uint8_t *sp,
				  uint16_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (sp == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	} else if (*sp == 0x00) {
		return NESEMU_RETURN_MEMORY_STACK_OVERFLOW;
	}
#endif
	// Reduce the sp (to fit u16)
	*sp -= 1;
	// Store value in stack
	nesemu_return_t err =
		nes_mem_w16(mem, NESEMU_STACK_GET_ADDR(*sp), value);
	// Reduce the sp (descending stack)
	*sp -= 1;
	return err;
}

nesemu_return_t nes_stack_pop_u16(struct nes_main_memory_t *mem,
				 uint8_t *sp,
				 uint16_t *result)
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
	// Increment the sp (to fix the extra byte)
	*sp += 1;
	// Read value from stack
	return nes_mem_r16(mem, addr, result);
}
