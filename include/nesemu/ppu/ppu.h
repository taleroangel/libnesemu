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
	uint16_t rasterline; /**< Index for the current horizontal pixel in scanline */
	uint8_t odd; /** Either 1 or 0, 1 on every odd frame */

    nes_ppu_oam_t oam; /** Primary OAM */
    nes_ppu_oam_t s_oam; /** Secondary OAM */
} nes_ppu_t;

/**
 * Initialize the PPU and its memory
 */
nesemu_return_t nes_ppu_init(struct nes_ppu *self,
			     struct nes_mem_main *mem);

/**
 * Render, exactly 1 pixel.
 * @note Rendered pixel might not be visible, as it also emulates HBLANK and VBLANK regions
 * 
 * @param display Reference to an array of RGB24 bytes
 * @param self PPU structure reference
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
