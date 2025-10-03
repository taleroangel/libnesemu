#include "nesemu/cpu/cpu.h"
#include "nesemu/cpu/instructions.h"
#include "nesemu/cpu/status.h"
#include "nesemu/memory/main.h"
#include "nesemu/util/error.h"

#include <stdint.h>
#include <string.h>

nesemu_return_t nes_cpu_init(struct nes_cpu *self,
			    struct nes_mem_main *mem)
{
	// Set every regsiter to 0, simulating booting from power off
	memset(self, 0, sizeof(struct nes_cpu));
	// Set the program counter to the addr at the CPU reset vector
	nes_mem_r16(mem, NESEMU_CPU_VECTOR_RESET, &self->pc);

	// SP will wraparound when result is negative (this is intentional!)
	self->sp -= (uint8_t)NESEMU_CPU_RESTART_SP;
	self->status = NESEMU_CPU_FLAGS_I;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_cpu_reset(struct nes_cpu *self,
			     struct nes_mem_main *mem)
{
	// Set the program counter to the addr at the CPU reset vector
	nes_mem_r16(mem, NESEMU_CPU_VECTOR_RESET, &self->pc);
	// Restart the stack pointer
	self->sp -= (uint8_t)NESEMU_CPU_RESTART_SP;

	// Only I flag gets set to 1, others are unchanged
	self->status |= NESEMU_CPU_FLAGS_I;

	return NESEMU_RETURN_SUCCESS;
}

inline uint8_t nes_cpu_fetch(struct nes_cpu *self,
			     struct nes_mem_main *mem)
{
	// Get next instruction, no fail
	uint8_t result;
	(void)nes_mem_r8(mem, self->pc++, &result);

	return result;
}
