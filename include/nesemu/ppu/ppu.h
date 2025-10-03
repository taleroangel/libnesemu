/**
 * NES PPU related code
 *
 * Reference
 * https://www.nesdev.org/wiki/PPU
 */

#ifndef __NESEMU_PPU_H__
#define __NESEMU_PPU_H__

#include "nesemu/util/error.h"
#include "nesemu/memory/main.h"
#include "nesemu/memory/video.h"

#include "palette.h"
#include "oam.h"

#include <stdint.h>

/** Visible screen height */
#define NESEMU_PPU_SCREEN_HEIGHT 240

/** Visible screen width */
#define NESEMU_PPU_SCREEN_WIDTH 256

/** Size (in bytes) of the framebuffer (RGB24) */
#define NESEMU_PPU_BUFFER_SIZE (NESEMU_PPU_SCREEN_HEIGHT * NESEMU_PPU_SCREEN_WIDTH * 3)

/** Type for the PPU image output (RGB24 buffer) */
typedef uint8_t nes_display_t[NESEMU_PPU_BUFFER_SIZE];

/**
 * Picture Processing Unit (NTSC only!)
 */
typedef struct nes_ppu {

	uint16_t scanline; /**< Index for the current scanline */
	uint16_t dot; /**< Index for the current dot in scanline */
	uint8_t frame; /**< Either 1 or 0, 1 on every odd frame */

    nes_ppu_palette_t *system_palette; /**< Reference to the system palette (RGB24) */

    struct nes_ppu_oam oam[NESEMU_PPU_OAM_SPRITES]; /**< Primary OAM */
    struct nes_ppu_oam s_oam[NESEMU_PPU_SOAM_SPRITES]; /**< Secondary OAM */

    uint8_t v; /**< Internal register */
    uint8_t t; /**< Internal register */
    uint8_t x; /**< Internal register */
    uint8_t w; /**< Internal register */

} nes_ppu_t;

/**
 * Initialize the PPU and its memory
 *
 * @param self PPU struct reference
 * @param system_palette Reference to system palette look-up table
 * @param mem Main system memory (for access to the PPU registers)
 *
 * @note A reference to the `system_palette` array will be stored inside
 * the ppu structure, keep this array in memory and alive as much as the
 * ppu structure.
 */
nesemu_return_t nes_ppu_init(struct nes_ppu *self,
			     nes_ppu_palette_t *system_palette,
			     struct nes_mem_main *mem);

/**
 * Render, exactly 1 pixel.
 * @note Rendered pixel might not be visible, as it also emulates HBLANK and VBLANK regions
 * 
 * @param self PPU structure reference
 * @param display Reference to an array of RGB24 bytes
 * @param mem System memory bus
 * @param vim Video memory bus
 * @param cycles Reference to an integer where the amount of PPU cycles the operation took
 */
nesemu_return_t nes_ppu_render(struct nes_ppu *self,
			       nes_display_t *display,
			       struct nes_mem_main *mem,
			       struct nes_mem_video *vim,
			       int *cycles);

/**
 * NES PPU registers
 *
 * Reference:
 * https://www.nesdev.org/wiki/PPU_registers
 */
enum nes_ppu_registers_t {
	NESEMU_PPU_REG_PPUCTRL = 0x2000,
	NESEMU_PPU_REG_PPUMASK = 0x2001,
	NESEMU_PPU_REG_PPUSTATUS = 0x2002,
	NESEMU_PPU_REG_OAMADDR = 0x2003,
	NESEMU_PPU_REG_OAMDATA = 0x2004,
	NESEMU_PPU_REG_PPUSCROLL = 0x2005,
	NESEMU_PPU_REG_PPUADDR = 0x2006,
	NESEMU_PPU_REG_PPUDATA = 0x2007,
	NESEMU_PPU_REG_OAMDMA = 0x4014,
};

#endif
