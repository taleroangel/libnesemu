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

/** Base memory address for nametable memory */
#define NESEMU_PPU_NAMETABLE_BASE_ADDR 0x2000

/** Memory offset for an attribute table */
#define NESEMU_PPU_ATTRTABLE_OFFSET 0x3C0

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
    if ((err = nes_mem_r8(mem, NESEMU_PPU_REG_PPUCTRL, &ppuctrl)) < NESEMU_RETURN_SUCCESS) {
        return err;
    }

    // Compute base nametable addr based on PPUCTRL first 2 bytes
    // (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
    uint16_t ntaddr = NESEMU_PPU_NAMETABLE_BASE_ADDR +
        (uint16_t)(ppuctrl & NESEMU_PPU_PPUCTRL_BASE_NAMETABLE) * NESEMU_PPU_NAMETABLE_OFFSET;

    // Base attribute table address
    uint16_t ataddr = ntaddr + NESEMU_PPU_ATTRTABLE_OFFSET;

    // Palette buffer (from attribute table)
    nes_vram_palette_t palette;

    // Pattern buffer
    uint8_t lo_bgpattern = 0;
    uint8_t hi_bgpattern = 0;

    /* Visible scanlines */
    if (self->scanline <= NESEMU_PPU_NTSC_RENDERING_SCANLINES) {
        
        // Get tile y coordinate in nametable
        int ytile = ((int)self->scanline / NESEMU_PPU_DOTS_PER_TILE);
        // Get pixel y coordinate in tile
        int ypix = ((int)self->scanline % NESEMU_PPU_DOTS_PER_TILE);

        // Foreach rasterline
        for (int x = 0; x < NESEMU_PPU_SCREEN_WIDTH; x++) {
            // Get tile x coordinate in nametable
            int xtile = (x / NESEMU_PPU_DOTS_PER_TILE);
            // Get pixel y coordinate in tile
            int xpix = (x % NESEMU_PPU_DOTS_PER_TILE);

            // Tile is buffered, read it only on the first dot of the new tile
            if (xpix == 0) {
                // Get index of tile
                int tidx = (NESEMU_PPU_NAMETABLE_HEIGHT * ytile) + xtile;
                // Get memory address of the tile
                uint16_t taddr = (ntaddr + tidx);
                // Buffer for tile data
                uint8_t tilebuff = 0;
                // Read tile data
                if ((err = nes_vram_r8(vim, taddr, &tilebuff)) < NESEMU_RETURN_SUCCESS) {
                    return err;
                }

                // Get attribute table for tile
                int attridx = (tidx / 2);
                // Get memory address for attribute
                uint16_t attraddr = (ataddr + attridx);
                // Buffer for atrribute data
                uint8_t attrbuff = 0;
                // Read attribute data
                if ((err = nes_vram_r8(vim, attraddr, &attrbuff)) < NESEMU_RETURN_SUCCESS) {
                    return err;
                }

                // Get cuadrant of current tile
                // value = (bottomright << 6) | (bottomleft << 4) | (topright << 2) | (topleft << 0)
                if (xtile % 2 > 0) {
                    attrbuff >>= 2;
                }
                if (ytile % 2 > 0) {
                    attrbuff >>= 4;
                }

                // Save palette (keep only 2 first bits)
                int paletteidx = (int)(attrbuff & 0x03);
                uint16_t paletteaddr = NESEMU_MEMORY_VRAM_PALETTE_ADDR + paletteidx;

                // Read palette
                if ((err = nes_vram_palette_read(vim, paletteaddr, &palette)) < NESEMU_RETURN_SUCCESS) {
                    return err;
                }

                // Get pattern table base addr
                uint16_t baddrbg = ((ppuctrl & NESEMU_PPU_PPUCTRL_BACKGROUND_PATTERN_TABLE) == 0)
                    ? 0x0000 : NESEMU_PPU_PATTERN_OFFSET; 

                // Get background pattern address
                uint16_t addrbg = baddrbg + (tidx * NESEMU_MEMORY_VRAM_PATTERN_SIZE);

                // Read pattern data
                nes_vram_pattern_t bgpattern;
                if ((err = nes_vram_pattern_read(vim, addrbg, &bgpattern)) < NESEMU_RETURN_SUCCESS) {
                    return err;
                }

                // Buffer pattern bitfields
                lo_bgpattern = bgpattern[ypix];
                hi_bgpattern = bgpattern[ypix + NESEMU_PPU_DOTS_PER_TILE];
            }

            // Decode color index from pattern data
            int bit = (7 - x);
            uint8_t lo = (lo_bgpattern >> bit) & 1;
            uint8_t hi = (hi_bgpattern >> bit) & 1;
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
        ;; /* Do nothing */
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
