#ifndef __NESEMU_MEMORY_VIDEO_H__
#define __NESEMU_MEMORY_VIDEO_H__

#include "nesemu/cartridge/cartridge.h"

#include <stdint.h>

/**
 * Real size for the PPU memory, excluding cartridge and mirroring
 */
#define NESEMU_MEMORY_VRAM_SIZE 0x20

/**
 * Size of the total addressable space.
 */
#define NESEMU_MEMORY_VRAM_ADDR_SIZE 0x4000

/**
 * Final address for the cartridge addressing (inclusive)
 */
#define NESEMU_MEMORY_VRAM_CARTRIDGE_END 0x3EFF

/**
 * Address modulo.
 *
 * Because the actual data starts from $0 instead of $3F00, a modulo is used.
 * This also works for mirrors.
 */
#define NESEMU_MEMORY_VRAM_BASE 0x20

/**
 * 16-bit addressable video memory.
 * Functions related to this memory type are named with `chr`
 */
struct nes_video_memory_t {
	/**
     * Raw memory array. Should not be accessed directly
     *
     * This contains only addresses mapped by the PPU directly ($3F00-$3F1F).
     * This is because this is the only real vram within the console hardware,
     * $0000-$3EFF is mapped to cartridge and $3F20-$3FFF is mirrored.
     * this is why the size of the array is smaller compared to the
     * addressable space.
     *
     * Memory addresses below $3F00 should delegate r/w operations to the
     * cartridge `ppu callbacks`.
     */
	uint8_t _data[NESEMU_MEMORY_VRAM_SIZE];

	/**
     * Game cartridge. Should already be initialized
     */
	struct nes_cartridge_t cartridge;
};

/**
 * Initialize memory to its initial state 
 */
nesemu_error_t nes_chr_reset(struct nes_video_memory_t *mem);

/**
 * Write 8 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param data Data to be pushed onto memory
 */
nesemu_error_t nes_chr_w8(struct nes_video_memory_t *mem,
			  uint16_t addr,
			  uint8_t data);

/**
 * Read 8 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param result Reference to where the result will be stored
 */
nesemu_error_t nes_chr_r8(struct nes_video_memory_t *mem,
			  uint16_t addr,
			  uint8_t *result);

/**
 * Write 16 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param data Data to be pushed onto memory
 */
nesemu_error_t nes_chr_w16(struct nes_video_memory_t *mem,
			   uint16_t addr,
			   uint16_t data);

/**
 * Read 16 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param result Reference to where the result will be stored
 */
nesemu_error_t nes_chr_r16(struct nes_video_memory_t *mem,
			   uint16_t addr,
			   uint16_t *result);

/**
 * Syntax sugar around cartridge reader
 */
static inline nesemu_error_t nes_chr_cartridge_read(
	struct nes_video_memory_t *mem,
	uint16_t addr,
	uint8_t *value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (mem->cartridge.chr_read_fn == NULL) {
		return NESEMU_RETURN_MEMORY_PRGROM_NO_DATA;
	}
#endif
	return mem->cartridge.chr_read_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(mem->cartridge), addr,
		value);
}

/**
 * Syntax sugar around cartridge writer
 */
static inline nesemu_error_t nes_chr_cartridge_write(
	struct nes_video_memory_t *mem,
	uint16_t addr,
	uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (mem->cartridge.chr_write_fn == NULL) {
		return NESEMU_RETURN_MEMORY_PRGROM_NO_DATA;
	}
#endif
	return mem->cartridge.chr_write_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(mem->cartridge), addr,
		value);
}

#endif
