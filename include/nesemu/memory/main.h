/**
 * References:
 * https://www.nesdev.org/wiki/CPU_memory_map
 */

#ifndef __NESEMU_MEMORY_MAIN_H__
#define __NESEMU_MEMORY_MAIN_H__

#include "nesemu/util/error.h"
#include "nesemu/cartridge/cartridge.h"

#include <stdint.h>

/**
 * Size of the CPU memory. Excluding cartridge
 */
#define NESEMU_MEMORY_MAIN_SIZE 0x4020

/**
 * Base memory mirroring range end (exclusive)
 */
#define NESEMU_MEMORY_RAM_MIRRORING_RANGE_START 0x07FF

/**
 * Base memory mirroring range end (inclusive)
 */
#define NESEMU_MEMORY_RAM_MIRRORING_RANGE_END 0x17FF

/**
 * Address mirroring base for addresses
 *      - 0x0000-0x07FF -> 0x0800-0x0FFF
 *      - 0x0800-0x0FFF
 *      - 0x1000-0x17FF
 */
#define NESEMU_MEMORY_RAM_MIRRORING_BASE 0x800

/**
 * 16-bit addressable main memory
 * Functions related to this memory type name it `mem`
 */
struct nes_main_memory_t {
	/**
     * Raw memory array. Should not be accessed directly.
     *
     * This only contains console internal memory structure.
     * Any fields mapped to cartridge are left out and r/w operations
     * are delegated to the cartridge structure instead.
     *
     * That is the reason why the size of this array is smaller than
     * the NES memory addressing
     */
	uint8_t _data[NESEMU_MEMORY_MAIN_SIZE];

	/**
     * Game cartridge. Should already be initialized
     */
	struct nes_cartridge_t cartridge;
};

/**
 * Initialize memory to its initial state 
 */
nesemu_error_t nes_mem_reset(struct nes_main_memory_t *mem);

/**
 * Write 8 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param data Data to be pushed onto memory
 */
nesemu_error_t nes_mem_w8(struct nes_main_memory_t *mem,
			  uint16_t addr,
			  uint8_t data);

/**
 * Read 8 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param result Reference to where the result will be stored
 */
nesemu_error_t nes_mem_r8(struct nes_main_memory_t *mem,
			  uint16_t addr,
			  uint8_t *result);

/**
 * Write 16 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param data Data to be pushed onto memory
 */
nesemu_error_t nes_mem_w16(struct nes_main_memory_t *mem,
			   uint16_t addr,
			   uint16_t data);

/**
 * Read 16 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param result Reference to where the result will be stored
 */
nesemu_error_t nes_mem_r16(struct nes_main_memory_t *mem,
			   uint16_t addr,
			   uint16_t *result);

/**
 * Syntax sugar around cartridge reader
 */
static inline nesemu_error_t nes_mem_cartridge_read(
	struct nes_main_memory_t *mem,
	uint16_t addr,
	uint8_t *value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (mem->cartridge.cpu_reader == NULL) {
		return NESEMU_RETURN_MEMORY_PRGROM_NO_DATA;
	}
#endif
	return mem->cartridge.cpu_reader(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(mem->cartridge), addr,
		value);
}

/**
 * Syntax sugar around cartridge writer
 */
static inline nesemu_error_t nes_mem_cartridge_write(
	struct nes_main_memory_t *mem,
	uint16_t addr,
	uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (mem->cartridge.cpu_writer == NULL) {
		return NESEMU_RETURN_MEMORY_PRGROM_NO_DATA;
	}
#endif
	return mem->cartridge.cpu_writer(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(mem->cartridge), addr,
		value);
}

#endif
