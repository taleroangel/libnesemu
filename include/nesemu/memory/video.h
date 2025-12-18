#ifndef __NESEMU_MEMORY_VIDEO_H__
#define __NESEMU_MEMORY_VIDEO_H__

#include "nesemu/cartridge/cartridge.h"

#include "nesemu/util/error.h"
#include <stdint.h>

/*
 * Size for the internal CIRAM.
 */
#define NESEMU_MEMORY_VRAM_CIRAM_SIZE 0x800 /* 2KiB */

/**
 * Size for the Palette RAM indexes
 */
#define NESEMU_MEMORY_VRAM_PALETTE_RAM_SIZE 0x20

/**
 * Size for a single palette
 */
#define NESEMU_MEMORY_VRAM_PALETTE_SIZE 4

/**
 * Size of the total addressable space.
 */
#define NESEMU_MEMORY_VRAM_ADDR_SIZE 0x4000

/**
 * Initial address for the Palette RAM indexes
 */
#define NESEMU_MEMORY_VRAM_PALETTE_ADDR 0x3F00

/**
 * Initial address for the CIRAM
 */
#define NESEMU_MEMORY_VRAM_CIRAM_ADDR 0x2000

/**
 * Pattern size in bytes
 */
#define NESEMU_MEMORY_VRAM_PATTERN_SIZE 16

/**
 * 16-bit addressable video memory (VRAM).
 * Functions related to this memory type are named with `chr`.
 *
 * This memory also acts as a bus for accessing both internal vram and CHR
 * memory within the cartridge so there is no need to worry about mappings or
 * mirroring when accessing vram through this structure's related methods.
 *
 * @note Pattern tables live in the cartridge, no memory allocated for them
 * here!.
 */
typedef struct nes_mem_video {
	/**
     * Console internal CIRAM (Nametables and Attribute Tables).
     * Address space between $2000-$2FFF.
     *
     * In the original NES hardware, this address space is mapped by the
     * cartridge. Because this structure acts as a bus, the mappings will be
     * handled by this structure's related methods instead.
     */
	uint8_t ciram[NESEMU_MEMORY_VRAM_CIRAM_SIZE];

	/**
     * Palette RAM indexes. Should not be accessed directly
     *
     * This contains only addresses mapped by the PPU directly ($3F00-$3F1F).
     * $0000-$3EFF is mapped by the cartridge and $3F20-$3FFF is mirrored.
     * this is why the size of the array is smaller compared to the
     * addressable space.
     *
     * Memory addresses below $3F00 should delegate r/w operations to the
     * cartridge `chr callbacks`.
     */
	uint8_t palette_ram[NESEMU_MEMORY_VRAM_PALETTE_RAM_SIZE];

	/**
     * Reference to the game cartridge. Should already be initialized
     *
     * Beware!, pattern table data ($0000-$1FFF) lives within cartridge's
     * CHRROM/CHRRAM section, r/w operations to those addresses will be 
     * delegated to the cartridge callbacks.
     */
	struct nes_cartridge *cartridge;

} nes_mem_video_t;

/**
 * Pattern raw data from pattern table
 */
typedef uint8_t nes_vram_pattern_t[NESEMU_MEMORY_VRAM_PATTERN_SIZE];

/**
 * A single palette in palette ram
 */
typedef uint8_t nes_vram_palette_t[NESEMU_MEMORY_VRAM_PALETTE_SIZE];

/**
 * Initialize memory to its initial state 
 */
nesemu_return_t nes_vram_init(struct nes_mem_video *self,
			      struct nes_cartridge *cartridge);

/**
 * Write 8 bits in memory at `addr`
 *
 * @param self Memory array
 * @param addr Memory address
 * @param data Data to be pushed onto memory
 */
nesemu_return_t nes_vram_w8(struct nes_mem_video *self,
			    uint16_t addr,
			    uint8_t data);

/**
 * Read 8 bits from memory at `addr`
 *
 * @param self Memory array
 * @param addr Memory address
 * @param result Reference to where the result will be stored
 */
nesemu_return_t nes_vram_r8(struct nes_mem_video *self,
			    uint16_t addr,
			    uint8_t *result);

/**
 * Write 16 bits in memory at `addr`
 *
 * @param self Memory array
 * @param addr Memory address (should not be last memory position)
 * @param data Data to be pushed onto memory
 */
nesemu_return_t nes_vram_w16(struct nes_mem_video *self,
			     uint16_t addr,
			     uint16_t data);

/**
 * Read 16 bits from memory at `addr`
 *
 * @param self Memory array
 * @param addr Memory address (should not be last memory position)
 * @param result Reference to where the result will be stored
 */
nesemu_return_t nes_vram_r16(struct nes_mem_video *self,
			     uint16_t addr,
			     uint16_t *result);

/**
 * Read raw pattern from a pattern table
 *
 * @param self Memory array
 * @param addr Memory address (should be in range for pattern tables)
 * @param result Reference to the pattern table array (should be allocated)
 */
nesemu_return_t nes_vram_pattern_read(struct nes_mem_video *self,
				      uint16_t addr,
				      nes_vram_pattern_t *pattern);

/**
 * Get a palette from palette RAM
 *
 * @param self Memory array
 * @param addr Memory address (should be in range for palette ram)
 * @param result Reference to the palette array (should be allocated)
 */
nesemu_return_t nes_vram_palette_read(struct nes_mem_video *self,
				      uint16_t addr,
				      nes_vram_palette_t *palette);

#endif
