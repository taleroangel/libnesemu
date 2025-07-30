#include "nesemu/cpu/cpu.h"
#include "nesemu/cpu/status.h"
#include "nesemu/util/error.h"

#include <stdint.h>
#include <string.h>

nesemu_error_t nes_cpu_init(struct nes_cpu_t *self)
{
	// Set every regsiter to 0, simulating booting from power off
	memset(self, 0, sizeof(struct nes_cpu_t));
	self->pc = NESEMU_CPU_RESET_VECTOR_ADDR;

	// SP will wraparound when result is negative (this is intentional!)
	self->sp -= (uint8_t)NESEMU_CPU_RESTART_SP;
	self->status = NESEMU_CPU_FLAGS_I;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_cpu_reset(struct nes_cpu_t *self)
{
	// Only the pc & sp registers restart, others are unchanged
	self->pc = NESEMU_CPU_RESET_VECTOR_ADDR;
	self->sp -= (uint8_t)NESEMU_CPU_RESTART_SP;

	// Only I flag gets set to 1, others are unchanged
	self->status |= NESEMU_CPU_FLAGS_I;

	return NESEMU_RETURN_SUCCESS;
}
