#ifndef __NESEMU_MEMORY_VIDEO_H__
#define __NESEMU_MEMORY_VIDEO_H__

#include "nesemu/cartridge/cartridge.h"

#include <stdint.h>

/*
 * Size for the internal CIRAM.
 */
#define NESEMU_MEMORY_VRAM_CIRAM_SIZE 0x800 /* 2KiB */

/**
 * Size for the Palette RAM indexes
 */
#define NESEMU_MEMORY_VRAM_PALETTE_SIZE 0x20

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
 * 16-bit addressable video memory (VRAM).
 * Functions related to this memory type are named with `chr`.
 *
 * This memory also acts as a bus for accessing both internal vram and CHR
 * memory within the cartridge so there is no need to worry about mappings or
 * mirroring when accessing vram through this structure's related methods.
 */
struct nes_video_memory_t {

    /**
     * Console internal CIRAM (Nametables and Attribute Tables).
     * Address space between $2000-$2FFF.
     *
     * In the original NES hardware, this address space is mapped by the
     * cartridge. Because this structure acts as a bus, the mappings will be
     * handled by this structure's related methods instead based of
     * `cartridge`'s informative flags `use_chram` and `external_vram`.
     *
     * If `cartridge.external_vram` is true this space will be unused
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
	uint8_t palette_ram[NESEMU_MEMORY_VRAM_PALETTE_SIZE];

	/**
     * Reference to the game cartridge. Should already be initialized
     */
	struct nes_cartridge_t *cartridge;
};

/**
 * Initialize memory to its initial state 
 */
nesemu_return_t nes_chr_init(struct nes_video_memory_t *self,
			    struct nes_cartridge_t *cartridge);

/**
 * Write 8 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param data Data to be pushed onto memory
 */
nesemu_return_t nes_chr_w8(struct nes_video_memory_t *self,
			  uint16_t addr,
			  uint8_t data);

/**
 * Read 8 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address
 * @param result Reference to where the result will be stored
 */
nesemu_return_t nes_chr_r8(struct nes_video_memory_t *self,
			  uint16_t addr,
			  uint8_t *result);

/**
 * Write 16 bits in memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param data Data to be pushed onto memory
 */
nesemu_return_t nes_chr_w16(struct nes_video_memory_t *self,
			   uint16_t addr,
			   uint16_t data);

/**
 * Read 16 bits from memory at `addr`
 *
 * @param mem Memory array
 * @param addr Memory address (should not be last memory position)
 * @param result Reference to where the result will be stored
 */
nesemu_return_t nes_chr_r16(struct nes_video_memory_t *memself,
			   uint16_t addr,
			   uint16_t *result);

#endif
