/**
 * References:
 * https://www.nesdev.org/wiki/CPU_ALL
 */

#ifndef __NESEMU_CPU_H__
#define __NESEMU_CPU_H__

#include "nesemu/cpu/status.h"
#include "nesemu/util/error.h"
#include "nesemu/memory/memory.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * When restarting the SP should be decreased
 * by this exact amount
 */
#define NESEMU_CPU_RESTART_SP 3

/**
 * Address to a pointer where the IRQ handler is
 */
#define NESEMU_CPU_IRQ_ADDR (uint16_t)0xFFFE

/**
 * 6502 CPU variant.
 * You should not access any of the registers directly.
 *
 * Reference for registers
 * https://www.nesdev.org/obelisk-6502-guide/registers.html
 *
 * @note This is a little-endian processor
 */
struct nes_cpu_t {
	/* Registers */
	uint16_t pc; /**< Program Counter */
	uint8_t sp; /**< Stack Pointer */
	uint8_t a; /**< Accumulator */
	uint8_t x; /**< Index Register X */
	uint8_t y; /**< Index Register Y */
	uint8_t status; /**< Processor Status */

	/* Support */
	bool stop; /**< Stop execution flag (use STP instruction) */
	uint8_t brk; /**< BRK Reason */
};

/**
 * Initialize CPU registers to their power-up state
 *
 * @note Initialize CPU last! after every other subsystem
 * @note Cartridge must be already loaded! Complete memory map is required
 */
nesemu_error_t nes_cpu_init(struct nes_cpu_t *self,
			    struct nes_main_memory_t *mem);

/**
 * Restart the CPU, like pressing the restart button
 */
nesemu_error_t nes_cpu_reset(struct nes_cpu_t *self,
			     struct nes_main_memory_t *mem);

/**
 * Process the next instruction (will only process 1 instruction and then return)
 *
 * @param self Reference to the CPU structure
 * @param mem System memory
 * @param cycles Reference to an integer where the amount of CPU cycles
 * for the decoded instruction will be stored
 *
 * @note Will execute the instruction at $PC and then return
 */
nesemu_error_t nes_cpu_next(struct nes_cpu_t *self,
			    struct nes_main_memory_t *mem,
			    int *cycles);

/**
 * Fetch the next word from memory at $pc and increment $pc
 */
uint8_t nes_cpu_fetch(struct nes_cpu_t *self, struct nes_main_memory_t *mem);

/**
 * Set CPU status register given a bit mask
 */
static inline void nes_cpu_status_mask_set(struct nes_cpu_t *self, uint8_t mask)
{
	self->status = NESEMU_CPU_STATUS_SET_MASK(self->status, mask);
}

/**
 * Remove CPU status flags given a bit mask
 */
static inline void nes_cpu_status_mask_unset(struct nes_cpu_t *self,
					     uint8_t mask)
{
	self->status = NESEMU_CPU_STATUS_UNSET_MASK(self->status, mask);
}

#endif
