#include "nesemu/ppu/ppu.h"
#include "nesemu/memory/main.h"
#include "nesemu/memory/video.h"
#include "nesemu/ppu/palette.h"
#include "nesemu/util/error.h"
#include <stdint.h>

/**
 * Framebuffer for video output
 *
 * @note Only for type punning.
 */
union nes_ppu_framebuffer {
	/** Framebuffer (RGB24 Format) as raw bytes */
	nes_display_t bytes;

	/** Framebuffer in RGB24 components representation */
	struct nes_ppu_rgb24 {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} __attribute__((packed))
	rgb24[NESEMU_PPU_SCREEN_HEIGHT * NESEMU_PPU_SCREEN_WIDTH];
};

/* --- Constants --- */

/** Number of vertical/horizontal pixels per nametable tile */
#define NESEMU_PPU_DOTS_PER_TILE 8

/** Number of horizontal/vertical tiles per attribute byte */
#define NESEMU_PPU_TILES_PER_ATTR 4

/** Number of horizontal/vertical tiles per attribute quadrant */
#define NESEMU_PPU_TILES_PER_QUAD 2

/** Memory offset for an attribute table */
#define NESEMU_PPU_ATTRTABLE_OFFSET 0x3C0

/** Size of the attrtable (row) */
#define NESEMU_PPU_ATTRTABLE_WIDTH 8

/** Two tiles per quadrant */
#define NESEMU_PPU_ATTRTABLE_QUADRANTS 2

/** Base memory address for nametable memory */
#define NESEMU_PPU_NAMETABLE_BASE_ADDR 0x2000

/** Nametable height (number of tiles in col) */
#define NESEMU_PPU_NAMETABLE_HEIGHT 30

/** Nametable width (number of tiles in row) */
#define NESEMU_PPU_NAMETABLE_WIDTH 32

/** Size of a nametable, useful for offsets */
#define NESEMU_PPU_NAMETABLE_OFFSET 0x400

/** Pattern table offset */
#define NESEMU_PPU_PATTERN_OFFSET 0x1000

/** Number of PPU dots/cycles per scanline */
#define NESEMU_PPU_NTSC_DOTS_PER_SCANLINE 341

/** Number of scanlines in NTSC format */
#define NESEMU_PPU_NTSC_SCANLINES 262

/** Last index for the last rendering scanline */
#define NESEMU_PPU_NTSC_RENDERING_SCANLINES 239

/** Index of the post-render scanline */
#define NESEMU_PPU_NTSC_IDLE_SCANLINE 240

/** Last index for the vertical blank section */
#define NESEMU_PPU_NTSC_VBLANK_SCANLINE 260

/** Index for the pre-render scanline */
#define NESEMU_PPU_NTSC_PRERENDER_SCANLINE 261

/* --- Function Definition --- */
nesemu_return_t nes_ppu_init(struct nes_ppu *self,
			     nes_ppu_system_palette_t *system_palette,
			     struct nes_mem_main *mem)
{
	nesemu_return_t err = NESEMU_RETURN_SUCCESS;

#ifndef NESEMU_DISABLE_SAFETY_CHECKS
	if (mem == NULL) {
		return NESEMU_RETURN_BAD_ARGUMENTS;
	}
	if (system_palette == NULL) {
		return NESEMU_RETURN_PPU_BAD_PALETTE;
	}
#endif

	// Set system palette
	self->system_palette = system_palette;

	// Start with the pre-render scanline (scanline -1)
	self->scanline = NESEMU_PPU_NTSC_PRERENDER_SCANLINE;

	return err;
}

nesemu_return_t nes_ppu_render(struct nes_ppu *self,
			       nes_display_t *display,
			       struct nes_mem_main *mem,
			       struct nes_mem_video *vim,
			       int *cycles)
{
	nesemu_return_t err = NESEMU_RETURN_SUCCESS;

	// Set number of cycles operation took
	*cycles = NESEMU_PPU_NTSC_DOTS_PER_SCANLINE;

	// Read PPUCTRL
	uint8_t ppuctrl;
	if ((err = nes_mem_r8(mem, NESEMU_PPU_REG_PPUCTRL, &ppuctrl)) <
	    NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Compute base nametable addr based on PPUCTRL first 2 bytes
	// (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
	uint16_t ntaddr =
		NESEMU_PPU_NAMETABLE_BASE_ADDR +
		(uint16_t)(ppuctrl & NESEMU_PPU_PPUCTRL_BASE_NAMETABLE) *
			NESEMU_PPU_NAMETABLE_OFFSET;

	// Base attribute table address
	uint16_t ataddr = ntaddr + NESEMU_PPU_ATTRTABLE_OFFSET;

	// Palette buffer (from attribute table)
	nes_vram_palette_t palette;

	// Pattern buffer
	uint8_t plane0 = 0;
	uint8_t plane1 = 0;

	/* Visible scanlines */
	if (self->scanline <= NESEMU_PPU_NTSC_RENDERING_SCANLINES) {
		// y tile coordinate
		int ycoarse = ((int)self->scanline / NESEMU_PPU_DOTS_PER_TILE);
		// y pixel coordinate (relative to tile)
		int yfine = ((int)self->scanline % NESEMU_PPU_DOTS_PER_TILE);

		// Foreach rasterline
		for (int x = 0; x < NESEMU_PPU_SCREEN_WIDTH; x++) {
			// x tile coordinate
			int xcoarse = (x / NESEMU_PPU_DOTS_PER_TILE);
			// x pixel coordinate (relative to tile)
			int xfine = (x % NESEMU_PPU_DOTS_PER_TILE);

			// Tile is buffered, read it only on the first dot of the tile
			if (xfine == 0) {
				// Get tile addr
				int tileidx = ycoarse * NESEMU_PPU_NAMETABLE_WIDTH + xcoarse;
				uint16_t taddr = ntaddr + tileidx;
				// Buffer for tile data
				uint8_t tilebuff = 0;
				if ((err = nes_vram_r8(vim, taddr, &tilebuff)) <
				    NESEMU_RETURN_SUCCESS) {
					return err;
				}

				// Get attribute table idx for tile
				// Each attribute byte covers a 4x4 tile block
				int attridx = 
                    (int)(ycoarse / NESEMU_PPU_TILES_PER_ATTR) * NESEMU_PPU_ATTRTABLE_WIDTH + 
                    (int)(xcoarse / NESEMU_PPU_TILES_PER_ATTR);

                // Attribute data is a byte with 4 quadrants
				uint16_t attraddr = (ataddr + attridx);
				uint8_t attrbuff = 0;
				if ((err = nes_vram_r8(vim, attraddr, &attrbuff)) < NESEMU_RETURN_SUCCESS) {
					return err;
				}

                // Each quadrant has an index to the palette to be used
                uint8_t quadx = (xcoarse % NESEMU_PPU_TILES_PER_ATTR) / NESEMU_PPU_TILES_PER_QUAD;
                uint8_t quady = (ycoarse % NESEMU_PPU_TILES_PER_ATTR) / NESEMU_PPU_TILES_PER_QUAD;
                uint8_t quadidx = quady * NESEMU_PPU_TILES_PER_QUAD + quadx;
                uint8_t paletteidx = (attrbuff >> (quadidx * NESEMU_PPU_TILES_PER_QUAD)) & 0x03;

                // Read palette data
				uint16_t paletteaddr = NESEMU_MEMORY_VRAM_PALETTE_ADDR + (paletteidx * NESEMU_MEMORY_VRAM_PALETTE_SIZE);
				if ((err = nes_vram_palette_read(vim, paletteaddr, &palette)) < NESEMU_RETURN_SUCCESS) {
					return err;
				}

				// Get pattern table base addr offset ($0000 or $1000)
				uint16_t bpttraddr = ((ppuctrl & NESEMU_PPU_PPUCTRL_BACKGROUND_PATTERN_TABLE) == 0)
                        ? 0x0000
                        : NESEMU_PPU_PATTERN_OFFSET;

				// Get background pattern
				uint16_t pttraddr = bpttraddr + NESEMU_MEMORY_VRAM_PATTERN_SIZE * tilebuff;
				nes_vram_pattern_t pttr;
				if ((err = nes_vram_pattern_read(vim, pttraddr, &pttr)) < NESEMU_RETURN_SUCCESS) {
					return err;
				}

				// Buffer pattern bitfields
				plane0 = pttr[yfine];
				plane1 = pttr[yfine + NESEMU_PPU_DOTS_PER_TILE];

			} // if (xfine == 0)

			// Decode color index from pattern data
			int bit = (7 - xfine);
			uint8_t lo = (plane0 >> bit) & 1;
			uint8_t hi = (plane1 >> bit) & 1;
			int bgidx = (hi << 1) | lo;

			// Get background color from palette
			int bgsysindex = palette[bgidx];
			nes_color_t bgcolor = (*self->system_palette)[bgsysindex];

			// Set color in display
			*display[(self->scanline * NESEMU_PPU_SCREEN_WIDTH) + x] = bgcolor;
		}
	}
	/* Idle section */
	else if (self->scanline == NESEMU_PPU_NTSC_IDLE_SCANLINE) {
		;
		; /* Do nothing */
	}
	/* VBlank */
	else if (self->scanline <= NESEMU_PPU_NTSC_VBLANK_SCANLINE) {
	}
	/* Pre-render */
	else if (self->scanline == NESEMU_PPU_NTSC_PRERENDER_SCANLINE) {
	}

	// Next scanline on rasterline completion, else keep same scanline
	self->scanline = (self->scanline + 1) % NESEMU_PPU_NTSC_SCANLINES;

	return err;
}
