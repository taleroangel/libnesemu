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
 * Size of the CPU memory. Excluding cartridge space
 */
#define NESEMU_MEMORY_RAM_SIZE 0x4020

/**
 * Initial address for the cartridge addressing
 */
#define NESEMU_MEMORY_RAM_CARTRIDGE_BEGIN 0x4020

/**
 * Base memory mirroring range end (inclusive)
 */
#define NESEMU_MEMORY_RAM_MIRRORING_RANGE_START 0x0800

/**
 * Base memory mirroring range end (inclusive)
 */
#define NESEMU_MEMORY_RAM_MIRRORING_RANGE_END 0x17FF

/**
 * Address mirroring base for modulo operator
 *                      -> 0x0000-0x07FF
 *      - 0x0800-0x0FFF
 *      - 0x1000-0x17FF
 *      - 0x1800-0x1FFF
 */
#define NESEMU_MEMORY_RAM_MIRRORING_BASE 0x800

/**
 * PPU register mirroring range start (inclusive)
 */
#define NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_RANGE_START 0x2008

/**
 * PPU register mirroring range end (inclusive)
 */
#define NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_RANGE_END 0x3FFF

/**
 * Address modulo for PPU register mirroring
 */
#define NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_BASE 0x8

/**
 * Base address for PPU register mirroring
 */
#define NESEMU_MEMORY_RAM_PPU_REG_MIRRORING_ADDR 0x2000

/**
 * 16-bit addressable memory. This is the console's main memory/CPU memory
 * This memory also acts as a bus for accessing both internal memory and PRG
 * memory within the cartridge so there is no need to worry about mappings or
 * mirroring when accessing memory through this structure's related methods.
 *
 * Functions related to this memory type are named with `mem`
 */
struct nes_main_memory_t {
	/**
     * Raw memory array. Should not be accessed directly.
     *
     * This only contains console internal memory structure ($0000-$401F).
     * Any fields mapped to cartridge are left out and r/w operations
     * are delegated to the cartridge structure callbacks instead.
     *
     * That is the reason why the size of this array is smaller than
     * the addressable space.
     *
     * Memory addresses above $401F should delegate r/w operations to the
     * cartridge `cpu callbacks`.
     *
     * OPTIMIZATION: Separate this memory in different sections to avoid
     * allocating memory to mirrored/unused addresses.
     */
	uint8_t _data[NESEMU_MEMORY_RAM_SIZE];

	/**
     * Reference to the game cartridge. Should already be initialized.
     */
	struct nes_cartridge_t *cartridge;
};

/**
 * Initialize memory to its initial state 
 */
nesemu_return_t nes_mem_init(struct nes_main_memory_t *self,
			    struct nes_cartridge_t *cartridge);

/**
 * Write 8 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param data Data to be pushed onto memory
 */
nesemu_return_t nes_mem_w8(struct nes_main_memory_t *self,
			  uint16_t addr,
			  uint8_t data);

/**
 * Read 8 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param result Reference to where the result will be stored
 */
nesemu_return_t nes_mem_r8(struct nes_main_memory_t *self,
			  uint16_t addr,
			  uint8_t *result);

/**
 * Write 16 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param data Data to be pushed onto memory
 */
nesemu_return_t nes_mem_w16(struct nes_main_memory_t *self,
			   uint16_t addr,
			   uint16_t data);

/**
 * Read 16 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param result Reference to where the result will be stored
 */
nesemu_return_t nes_mem_r16(struct nes_main_memory_t *self,
			   uint16_t addr,
			   uint16_t *result);

#endif
