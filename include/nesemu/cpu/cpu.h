/**
 * References:
 * https://www.nesdev.org/wiki/CPU_ALL
 */

#ifndef __NESEMU_CPU_H__
#define __NESEMU_CPU_H__

#include "nesemu/util/error.h"
#include "nesemu/memory/memory.h"

#include <stdint.h>

/**
 * Power up state for PC register
 */
#define NESEMU_CPU_INIT_PC 0xFFFC

/**
 * When restarting the SP should be decreased
 * by this exact amount
 */
#define NESEMU_CPU_RESTART_SP 3

/**
 * 6502 CPU variant
 *
 * Reference for registers
 * https://www.nesdev.org/obelisk-6502-guide/registers.html
 *
 * @note This is a little-endian processor
 */
struct nes_cpu_t {
	uint16_t pc; /**< Program Counter */
	uint8_t sp; /**< Stack Pointer */
	uint8_t acc; /**< Accumulator */
	uint8_t irx; /**< Index Register X */
	uint8_t iry; /**< Index Register Y */
	uint8_t status; /**< Processor Status */
};

/**
 * Initialize CPU registers to their power-up state
 */
nesemu_error_t nes_cpu_init(struct nes_cpu_t *self);

/**
 * Restart the CPU, like pressing the restart button
 */
nesemu_error_t nes_cpu_reset(struct nes_cpu_t *self);

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
nesemu_error_t
nes_cpu_next(struct nes_cpu_t *self, nes_memory_t mem, int *cycles);

#endif
