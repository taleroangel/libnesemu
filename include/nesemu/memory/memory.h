/**
 * References:
 * https://www.nesdev.org/wiki/CPU_memory_map
 */

#ifndef __NESEMU_MEMORY_H__
#define __NESEMU_MEMORY_H__

#include "nesemu/util/error.h"

#include <stdint.h>

/**
 * Memory size for a 16-bit addressable memory
 */
#define NESEMU_MEMORY_SIZE 0x10000 /* 0xFFFF + 1 */

/**
 * 16-bit addressable memory
 */
typedef uint8_t nes_memory_t[NESEMU_MEMORY_SIZE];

/**
 * Write 8 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param data Data to be pushed onto memory
 */
nesemu_error_t nes_mem_w8(nes_memory_t mem, uint16_t addr, uint8_t data);

/**
 * Read 8 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param result Reference to where the result will be stored
 */
nesemu_error_t nes_mem_r8(nes_memory_t mem, uint16_t addr, uint8_t *result);

/**
 * Write 16 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param data Data to be pushed onto memory
 */
nesemu_error_t nes_mem_w16(nes_memory_t mem, uint16_t addr, uint16_t data);

/**
 * Read 16 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param result Reference to where the result will be stored
 */
nesemu_error_t nes_mem_r16(nes_memory_t mem, uint16_t addr, uint16_t *result);

#endif
